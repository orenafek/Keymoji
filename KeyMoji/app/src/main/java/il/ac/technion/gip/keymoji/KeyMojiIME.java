package il.ac.technion.gip.keymoji;

import android.graphics.drawable.Drawable;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.media.AudioManager;
import android.os.Parcel;
import android.support.annotation.DrawableRes;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.InputConnection;
import android.view.textservice.SentenceSuggestionsInfo;
import android.view.textservice.SpellCheckerSession;
import android.view.textservice.SuggestionsInfo;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import AndroidAuxilary.Inflater;
import io.github.rockerhieu.emojicon.emoji.Emojicon;

import static io.github.rockerhieu.emojicon.emoji.Emojicon.TYPE_PEOPLE;

/**
 * @author Oren Afek
 * @since 27/02/17
 */

public class KeyMojiIME extends InputMethodService implements SpellCheckerSession.SpellCheckerSessionListener {

    private static final List<Emojicon> allEmojis;
    private static final List<String> allEmojisStrings;
    private SparseArray<String> emojis;

    static {
        allEmojis = new ArrayList<>();
        allEmojisStrings = new ArrayList<>();
        int[] emojiTypes = new int[]{
                TYPE_PEOPLE/*, TYPE_NATURE, TYPE_OBJECTS, TYPE_PLACES, TYPE_SYMBOLS*/};
        for (int type : emojiTypes) {
            allEmojis.addAll(Arrays.asList(Emojicon.getEmojicons(type)));
        }

        for (Emojicon emoji : allEmojis) {
            allEmojisStrings.add(emoji.getEmoji());
        }
    }

    private int lastDisplayWidth;
    private boolean isCompletionOn = false;
    private SpellCheckerSession spellCheckSession;
    private StringBuilder composing = new StringBuilder();
    private List<String> suggestions;
    private CandidateView candidateView;

    @Override
    public void onInitializeInterface() {
        if (keyboard != null) {
            // Configuration changes can happen after the keyboard gets recreated,
            // so we need to be able to re-build the keyboards if the available
            // space has changed.
            int displayWidth = getMaxWidth();
            if (displayWidth == lastDisplayWidth) return;
            lastDisplayWidth = displayWidth;
        }
    }

    private boolean isEmoji(int primaryCode) {
        List<String> emojis = Arrays.asList(getResources().getStringArray(R.array.emoji_primary_codes));
        return emojis.contains(String.valueOf(primaryCode));
    }


    private KeyboardView keyboardView;
    private Keyboard keyboard;

    private boolean capsLock = false;

    @Override
    public View onCreateInputView() {
        keyboardView = new Inflater(this).inflate(R.layout.keyboard);
        keyboard = new Keyboard(this, R.xml.qwerty);
        registerEmoji(keyboard, R.drawable.emoji_1f618);
        keyboardView.setKeyboard(keyboard);
        emojis = initializeEmojisMap();
        keyboardView.setOnKeyboardActionListener(new KeyboardView.OnKeyboardActionListener() {
            @Override
            public void onPress(int primaryCode) {

            }

            @Override
            public void onRelease(int primaryCode) {

            }

            @Override
            public void onKey(int primaryCode, int[] keyCodes) {
                InputConnection ic = getCurrentInputConnection();
                playClick(primaryCode);
                switch (primaryCode) {
                    case Keyboard.KEYCODE_DELETE:
                        ic.deleteSurroundingText(1, 0);
                        break;
                    case Keyboard.KEYCODE_SHIFT: {
                        capsLock = !capsLock;
                        keyboard.setShifted(capsLock);
                        keyboardView.invalidateAllKeys();
                    }
                    break;
                    case Keyboard.KEYCODE_DONE:
                        ic.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_ENTER));
                        break;
                    default: {
                        if (isEmoji(primaryCode)) {
                            ic.commitText(emojis.get(primaryCode), 1);
                        } else {
                            char c = (char) primaryCode;
                            ic.commitText(String.valueOf(Character.isLetter(c) && !capsLock ?
                                    c : Character.toUpperCase(c)), 1);
                        }

                        composing.append((char) primaryCode);
                        updateCandidates();
                    }

                }


            }

            @Override
            public void onText(CharSequence text) {

            }

            @Override
            public void swipeLeft() {

            }

            @Override
            public void swipeRight() {

            }

            @Override
            public void swipeDown() {

            }

            @Override
            public void swipeUp() {

            }
        });
        return keyboardView;
    }

    private SparseArray<String> initializeEmojisMap() {
        SparseArray<String> map = new SparseArray<>();
        int[] unicodes = getResources().getIntArray(R.array.emojis_unicode);
        String[] primaryCodes = getResources().getStringArray(R.array.emoji_primary_codes);
        if (BuildConfig.DEBUG && unicodes.length != primaryCodes.length) {
            throw new RuntimeException();
        }
        for (int i = 0; i < unicodes.length; i++) {
            map.put(Integer.parseInt(primaryCodes[i]), new String(Character.toChars(unicodes[i])));
        }

        return map;
    }

    @Override
    public void onDisplayCompletions(CompletionInfo[] completions) {
        super.onDisplayCompletions(completions);
        System.out.println("");

    }

    @Override
    public View onCreateCandidatesView() {
        super.onCreateCandidatesView();
        candidateView = new CandidateView(this);
        candidateView.setService(this);
        return candidateView;
    }


    private void playClick(int primaryCode) {
        AudioManager am = (AudioManager) getSystemService(AUDIO_SERVICE);
        am.setRingerMode(AudioManager.RINGER_MODE_VIBRATE);
        switch (primaryCode) {
            case 32:
                am.playSoundEffect(AudioManager.FX_KEYPRESS_SPACEBAR);
                break;
            case Keyboard.KEYCODE_DONE:
            case 10:
                am.playSoundEffect(AudioManager.FX_KEYPRESS_RETURN);
                break;
            case Keyboard.KEYCODE_DELETE:
                am.playSoundEffect(AudioManager.FX_KEYPRESS_DELETE);
                break;
            default:
                am.playSoundEffect(AudioManager.FX_KEYPRESS_STANDARD);
        }
    }

    private int getEmojiCode(@DrawableRes int emojiRes) {
        for (Field f : io.github.rockerhieu.emojicon.R.drawable.class.getFields()) {
            try {
                String name = f.getName();
                if (name.contains("emoji") && f.getInt(null) == emojiRes) {
                    name = name.replace("emoji_", "").replace("_", "");
                    return Integer.parseInt(name, name.matches(".*[A-Za-z]+.*") ? 16 : 10);
                }
            } catch (IllegalAccessException | NumberFormatException ignore) {/**/}
        }

        return -1;
    }

    private void registerEmoji(Keyboard keyboard, @DrawableRes int emojiRes) {
        List<Keyboard.Key> keys = keyboard.getKeys();
        Keyboard.Row row = new Keyboard.Row(keyboard);
        row.defaultHeight = 80;

        Drawable emoji = getResources().getDrawable(emojiRes);

        Emojicon emojicon = Emojicon.fromResource(emojiRes, 0);
        Parcel p = Parcel.obtain();
        emojicon.writeToParcel(p, 0);
        Keyboard.Key key = new Keyboard.Key(row);
        key.codes = new int[]{emojiRes};
        key.gap = 10;
        key.height = row.defaultHeight = emoji.getMinimumHeight();
        key.width = row.defaultWidth = emoji.getMinimumWidth();
        key.icon = emoji;
        keys.add(key);
    }

    private void changeKeyMode(int primaryKey, Keyboard keyboard) {
        int index = primaryKey + Integer.parseInt(getString(R.string.emoji_anger));
        keyboard.getKeys().get(index).height = 0;
        keyboard.getKeys().get(index).width = 0;
    }

    public void pickSuggestionManually(int suggestion) {

    }

    @Override
    public void onFinishInput() {
        super.onFinishInput();

        updateCandidates();
    }

    private void updateCandidates() {
        if (!isCompletionOn) {
            if (composing.length() > 0) {
                setSuggestions(Arrays.asList(emojis.get(-54), emojis.get(-55)), true, true);
            } else {
                setSuggestions(null, false, false);
            }
        }

    }

    private void setSuggestions(List<String> suggestions, boolean completions, boolean typedWordValid) {
        if ((suggestions != null && suggestions.size() > 0) || isExtractViewShown()) {
            setCandidatesViewShown(true);
        }
        this.suggestions = suggestions;
        if (this.candidateView != null) {
            this.candidateView.setSuggestions(suggestions, completions, typedWordValid);
        }
    }

    @Override
    public void onGetSuggestions(SuggestionsInfo[] results) {

    }

    @Override
    public void onGetSentenceSuggestions(SentenceSuggestionsInfo[] results) {

    }
}
