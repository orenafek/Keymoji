package ch.hepia.iti.opencvnativeandroidstudio;

import android.graphics.drawable.Drawable;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.media.AudioManager;
import android.os.Parcel;
import android.support.annotation.DrawableRes;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.InputConnection;

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

public class KeyMojiIME extends InputMethodService {

    private static final List<Emojicon> allEmojis;
    private static final List<String> allEmojisStrings;

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

    private boolean isEmoji(int primaryCode) {
        int emojiCode = getEmojiCode(primaryCode);
        return emojiCode != -1 && allEmojis.contains(Emojicon.fromCodePoint(emojiCode));
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
                            ic.commitText(Emojicon.fromCodePoint
                                    (getEmojiCode(primaryCode)).getEmoji(), 1);
                        } else {
                            char c = (char) primaryCode;
                            ic.commitText(String.valueOf(Character.isLetter(c) && capsLock ?
                                    c : Character.toUpperCase(c)), 1);
                        }
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

    private void playClick(int primaryCode) {
        AudioManager am = (AudioManager) getSystemService(AUDIO_SERVICE);
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

        Drawable emoji = getResources().getDrawable(emojiRes);

        Emojicon emojicon = Emojicon.fromResource(emojiRes, 0);
        Parcel p = Parcel.obtain();
        emojicon.writeToParcel(p, 0);
        Keyboard.Key key = new Keyboard.Key(row);
        key.codes = new int[]{emojiRes};
        key.gap = 10;
        key.height = emoji.getMinimumHeight();
        key.width = emoji.getMinimumWidth();
        key.icon = emoji;
        keys.add(key);
    }
}
