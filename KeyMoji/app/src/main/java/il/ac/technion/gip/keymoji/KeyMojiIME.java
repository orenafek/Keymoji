package il.ac.technion.gip.keymoji;

import android.Manifest;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.media.AudioManager;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.InputConnection;
import android.view.textservice.SentenceSuggestionsInfo;
import android.view.textservice.SpellCheckerSession;
import android.view.textservice.SuggestionsInfo;

import com.permissioneverywhere.PermissionEverywhere;
import com.permissioneverywhere.PermissionResponse;
import com.permissioneverywhere.PermissionResultCallback;

import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.core.Mat;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import AndroidAuxilary.Inflater;
import AndroidAuxilary.Wrapper;
import io.github.rockerhieu.emojicon.emoji.Emojicon;

import static AndroidAuxilary.AssetCopier.copyAssetFolder;
import static il.ac.technion.gip.keymoji.KeyMojiIME.ACTION.NOTHING;
import static io.github.rockerhieu.emojicon.emoji.Emojicon.TYPE_PEOPLE;

/**
 * @author Oren Afek
 * @since 27/02/17
 */

public class KeyMojiIME extends InputMethodService implements SpellCheckerSession.SpellCheckerSessionListener {

    private static final List<Emojicon> allEmojis;
    private static final List<String> allEmojisStrings;
    private static final int REQ_CODE = 99999;
    private final Wrapper<Integer> result = new Wrapper<>(-1);
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
    private StringBuilder composing = new StringBuilder();
    private List<String> suggestions;
    private CandidateView candidateView;
    private JavaCameraView camera;

    @Override
    public void onCreate() {
        super.onCreate();
        // Permissions for Android 6+
        PermissionEverywhere.getPermission(getApplicationContext(),
                new String[]{Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE},
                REQ_CODE,
                "Notification title",
                "This app needs a write permission",
                R.mipmap.ic_launcher)
                .enqueue(new PermissionResultCallback() {
                    @Override
                    public void onComplete(PermissionResponse permissionResponse) {
                    }
                });

        String toPath = "/data/data/" + getPackageName();  // Your application path
        copyAssetFolder(getAssets(), "", toPath);


        // Load ndk built module, as specified
        // in moduleName in build.gradle
        System.loadLibrary("native-lib");

        camera = new JavaCameraView(getApplicationContext(), 3);
        camera.setCvCameraViewListener(new CameraListener());

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        disableCamera();
    }

    private void disableCamera() {
        camera.disableView();
    }


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

    @Override
    public void onFinishInput() {
        super.onFinishInput();
        composing.setLength(0);
        updateCandidates();
        setCandidatesViewShown(false);

    }

    private void updateCandidates() {
        if (!isCompletionOn) {
            if (composing.length() > 0) {
                synchronized (result) {
                    if (!result.isDefaulted()) {
                        setSuggestions(Collections.singletonList(emojis.get(result.get())), true, true);
                    } else {
                        setSuggestions(null, false, false);
                    }
                }
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

    private class CameraListener implements CameraBridgeViewBase.CvCameraViewListener2 {

        private int frameCounter = 0;


        @Override
        public void onCameraViewStarted(int width, int height) {
            DO(NOTHING);
        }

        @Override
        public void onCameraViewStopped() {
            DO(NOTHING);
        }

        @Override
        public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
            synchronized (result) {
                result.setDefault();
            }
            Mat matGray = inputFrame.gray();
            if (frameCounter % 30 == 0) {
                /* TODO: Flip image
                  FIXME: Core.flip(matGray.t(), matGray, 1);
                */
                int suggestion = getEmotion(matGray.getNativeObjAddr());
                if (suggestion != 0) {
                    synchronized (result) {
                        result.set(suggestion);
                    }
                }

            }
            frameCounter++;
            return matGray;
        }
    }

    public native int getEmotion(long matAddrGray);

    @Override
    public void onGetSuggestions(SuggestionsInfo[] results) {

    }

    @Override
    public void onGetSentenceSuggestions(SentenceSuggestionsInfo[] results) {

    }

    private void DO(ACTION __) {
    }

    enum ACTION {NOTHING}

    public void pickSuggestionManually(int mSelectedIndex) {
        DO(NOTHING);
    }
}
