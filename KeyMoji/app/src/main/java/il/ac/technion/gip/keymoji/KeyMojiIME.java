package il.ac.technion.gip.keymoji;

import android.Manifest;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.media.AudioManager;
import android.util.SparseArray;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.textservice.SentenceSuggestionsInfo;
import android.view.textservice.SpellCheckerSession;
import android.view.textservice.SuggestionsInfo;
import android.widget.FrameLayout;
import android.widget.Toast;

import com.google.android.cameraview.CameraView;
import com.intentfilter.androidpermissions.PermissionManager;
import com.permissioneverywhere.PermissionEverywhere;
import com.permissioneverywhere.PermissionResponse;
import com.permissioneverywhere.PermissionResultCallback;

import org.opencv.android.Utils;
import org.opencv.core.Mat;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import AndroidAuxilary.Inflater;
import AndroidAuxilary.ViewAccessor;
import AndroidAuxilary.Wrapper;

import static AndroidAuxilary.AssetCopier.copyAssetFolder;
import static il.ac.technion.gip.keymoji.KeyMojiIME.ACTION.NOTHING;
import static java.util.Collections.singleton;
/**
 * @author Oren Afek
 * @since 27/02/17
 */

public class KeyMojiIME extends InputMethodService implements SpellCheckerSession.SpellCheckerSessionListener,
        KeyboardView.OnKeyboardActionListener {


    private static final int REQ_CODE = 99999;
    private final Wrapper<Integer> result = new Wrapper<>(-1);
    private Inflater inflater;
    private CameraView cameraView;
    private SparseArray<String> emojis;
    private int lastDisplayWidth;
    private boolean isCompletionOn = false;
    private StringBuilder composing = new StringBuilder();
    private List<String> suggestions;
    private CandidateView candidateView;
    private FrameLayout mainLayout;
    private KeyboardView keyboardView;
    private Keyboard keyboard;
    private boolean capsLock = false;
    private ViewAccessor viewAccessor;



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
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        disableCamera();
    }

    private void disableCamera() {
        /*cameraView.disableView();*/
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
            return;
        }

        keyboard = new Keyboard(this, R.xml.qwerty);

    }

    private boolean isEmoji(int primaryCode) {
        List<String> emojis = Arrays.asList(getResources().getStringArray(R.array.emoji_primary_codes));
        return emojis.contains(String.valueOf(primaryCode));
    }

    @Override
    public View onCreateInputView() {
        this.inflater = new Inflater(this);
        mainLayout = inflater.inflate(R.layout.keyboard);
        viewAccessor = new ViewAccessor(mainLayout);
        keyboardView = (KeyboardView) mainLayout.getChildAt(0);
        keyboardView.setPreviewEnabled(false);
        cameraView = viewAccessor.getView(R.id.camera);
        cameraView.addCallback(new CameraView.Callback() {
            @Override
            public void onCameraOpened(CameraView cameraView) {
                super.onCameraOpened(cameraView);
            }

            @Override
            public void onCameraClosed(CameraView cameraView) {
                super.onCameraClosed(cameraView);
            }

            @Override
            public void onPictureTaken(CameraView cameraView, byte[] data) {
                super.onPictureTaken(cameraView, data);
                Bitmap b = BitmapFactory.decodeByteArray(data, 0, data.length);
                Mat m = new Mat();
                Utils.bitmapToMat(b, m);
                int suggestion = A(m.getNativeObjAddr());
                if (suggestion != 0) {
                    synchronized (result) {
                        result.set(suggestion);
                        updateCandidates();
                    }
                }

                System.out.println("");

            }
        });
        PermissionManager permissionManager = PermissionManager.getInstance(getApplicationContext());
        permissionManager.checkPermissions(singleton(Manifest.permission.CAMERA),
                new PermissionManager.PermissionRequestListener() {
            @Override
            public void onPermissionGranted() {

            }

            @Override
            public void onPermissionDenied() {
                Toast.makeText(getApplicationContext(), "Permissions Denied", Toast.LENGTH_SHORT).show();
            }
                });

        keyboardView.setKeyboard(keyboard);
        emojis = initializeEmojisMap();
        keyboardView.setOnKeyboardActionListener(this);

        new Timer().scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                //cameraView.takePicture();
            }
        }, 5, 3000);// First time start after 5 mili second and repead after 1 second
        return mainLayout;
    }


    public native int A(long add);
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

    public native int getEmotion(long matAddrGray);

    @Override
    public void onStartInput(EditorInfo attribute, boolean restarting) {
        super.onStartInput(attribute, restarting);
        DO(NOTHING);
    }

    @Override
    public void onStartInputView(EditorInfo info, boolean restarting) {
        super.onStartInputView(info, restarting);
        keyboardView.closing();
    }

    @Override
    public void onGetSuggestions(SuggestionsInfo[] results) {

    }

    @Override
    public void onGetSentenceSuggestions(SentenceSuggestionsInfo[] results) {

    }

    private void DO(ACTION __) {
    }

    public void pickSuggestionManually(int mSelectedIndex) {
        DO(NOTHING);
        sendText(indexToEmoji(mSelectedIndex));
    }

    enum ACTION {NOTHING}

    private CharSequence indexToEmoji(int relativeIndex) {
        return emojis.get(-54 + relativeIndex);
    }

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

            case KeyMojiKeyboard.KEYCODE_LANGUAGE_SWITCH:


            default: {
                if (isEmoji(primaryCode)) {
                    sendText(emojis.get(primaryCode));
                } else {
                    char c = (char) primaryCode;
                    sendText(String.valueOf(Character.isLetter(c) && !capsLock ?
                            c : Character.toUpperCase(c)));
                }

                composing.append((char) primaryCode);
                updateCandidates();
                setSuggestions(Collections.singletonList(emojis.get((-54))), true, true);
            }

        }

    }

    private void sendText(CharSequence cs) {
        InputConnection ic = getCurrentInputConnection();
        ic.commitText(cs, 1);
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


}
