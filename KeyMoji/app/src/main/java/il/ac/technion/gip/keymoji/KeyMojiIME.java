package il.ac.technion.gip.keymoji;

import android.Manifest;
import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.content.res.TypedArray;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.hardware.Camera;
import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.os.Environment;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.CompletionInfo;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputConnection;
import android.view.inputmethod.InputMethodManager;
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

import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.imgcodecs.Imgcodecs;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Timer;
import java.util.TimerTask;

import AndroidAuxilary.Emoji;
import AndroidAuxilary.Inflater;
import AndroidAuxilary.ViewAccessor;
import AndroidAuxilary.Wrapper;

import static AndroidAuxilary.AssetCopier.copyAssetFolder;
import static android.hardware.Camera.CameraInfo.CAMERA_FACING_FRONT;
import static il.ac.technion.gip.keymoji.KeyMojiIME.ACTION.NOTHING;
import static il.ac.technion.gip.keymoji.R.id.camera;

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
    private List<Emoji> emojis;
    private int lastDisplayWidth;
    private boolean isCompletionOn = false;
    private StringBuilder composing = new StringBuilder();
    private List<Emoji> suggestions;
    private CandidateView candidateView;
    private FrameLayout mainLayout;
    private CustomizedKeyboardView keyboardView;
    private KeyboardManager keyboardManager = new KeyboardManager();
    private boolean capsLock = false;
    private ViewAccessor viewAccessor;
    private final Wrapper<Boolean> takePicture = new Wrapper<>(false);
    private Camera.Size sz;
    boolean accessGranted;
    boolean timerInitialized;

    @Override
    public void onCreate() {
        super.onCreate();
        accessGranted = false;
        timerInitialized = false;


        // Permissions for Android 6+
        PermissionEverywhere.getPermission(getApplicationContext(),
                new String[]{Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE},
                REQ_CODE,
                "KeyMoji",
                "Allow camera and storage access",
                R.mipmap.ic_launcher)
                .enqueue(new PermissionResultCallback() {


                    @Override
                    public void onComplete(PermissionResponse permissionResponse) {
                        accessGranted = true;
                        Camera mCamera = null;
                        try {
                            mCamera = Camera.open(CAMERA_FACING_FRONT);
                        } catch (Exception e) {
                            Log.e("****************Error", e.toString());
                            Log.e("****************Error", "Camera service problem :(");
                        }

                        Camera.Parameters params = mCamera.getParameters();
                        sz = params.getPictureSize();

                        new Timer().schedule(new TimerTask() {
                            @Override
                            public void run() {
                                try {
                                    if (cameraView.isCameraOpened())
                                        cameraView.takePicture();
                                } catch (RuntimeException re) {
                                    Log.i("Daniel", "take picture failed :(");
                                }
                            }
                        }, 6000, 5000);
                        timerInitialized = true;

                        Log.e("***************Finished", "Camera service success");


                    }
                });

        String toPath = "/data/data/" + getPackageName();  // Your application path
        copyAssetFolder(getAssets(), "", toPath);

//        Camera mCamera = null;
//        try {
//            mCamera = Camera.open(CAMERA_FACING_FRONT);
//        } catch(Exception e){
//            Log.e("****************Error",e.toString());
//            Log.e("****************Error","Camera service problem :(");
//        }
//
//        Camera.Parameters params = mCamera.getParameters();
//        sz = params.getPictureSize();



        // Load ndk built module, as specified
        // in moduleName in build.gradle
        System.loadLibrary("native-lib");

        if (accessGranted && !timerInitialized) {
            new Timer().schedule(new TimerTask() {
                @Override
                public void run() {
                    try {
                        if (cameraView.isCameraOpened())
                            cameraView.takePicture();
                    } catch (RuntimeException re) {
                        Log.i("Daniel", "take picture failed :(");
                    }
                }
            }, 6000, 5000);
            timerInitialized = true;
        }

        if (this.candidateView != null && !accessGranted)
            setSuggestions(Collections.singletonList(new Emoji(999, "authorize camera from notifications")), true, true);

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (accessGranted)
            disableCamera();
    }

    private void disableCamera() {
        if (accessGranted)
            cameraView.stop();
    }

    @Override
    public void onInitializeInterface() {
        if (keyboardManager != null && keyboardManager.active != null) {
            // Configuration changes can happen after the qwerty gets recreated,
            // so we need to be able to re-build the keyboards if the available
            // space has changed.
            int displayWidth = getMaxWidth();
            if (displayWidth == lastDisplayWidth) return;
            lastDisplayWidth = displayWidth;
            return;
        }

        keyboardManager.initKeyboards();

    }


    private boolean isEmoji(CharSequence cs) {
        if (cs.length() < 2) {
            return false;
        }

        for (Emoji e : emojis) {
            for (String s : e.getEmojiStrings())
                if (s.equals(cs)) {
                    return true;
            }
        }


        return false;
    }

    private void sendEmoji(Emoji e, int relativeIndex) {
        if (accessGranted)
            sendText(e.getEmojiStrings().get(relativeIndex >= 0 ? relativeIndex : 0));

    }

    @Override
    public boolean onShowInputRequested(int flags, boolean configChange) {
        if (accessGranted && cameraView != null && !cameraView.isCameraOpened()) {
            cameraView.start();
        }

        // notify the user to allow access to the camera
        if (this.candidateView != null && !accessGranted)
            setSuggestions(Collections.singletonList(new Emoji(999, "authorize camera from notifications")), true, true);
        if (accessGranted && !timerInitialized) {
            new Timer().schedule(new TimerTask() {
                @Override
                public void run() {
                    try {
                        if (cameraView.isCameraOpened())
                            cameraView.takePicture();
                    } catch (RuntimeException re) {
                        Log.i("Daniel", "take picture failed :(");
                    }
                }
            }, 6000, 5000);
            timerInitialized = true;
        }

        return super.onShowInputRequested(flags, configChange);
    }

    @Override
    public View onCreateInputView() {
        this.inflater = new Inflater(this);
        mainLayout = inflater.inflate(R.layout.keyboard);
        viewAccessor = new ViewAccessor(mainLayout);
        keyboardView = (CustomizedKeyboardView) (mainLayout.getChildAt(0));
        keyboardView.setPreviewEnabled(false);
        cameraView = viewAccessor.getView(camera);
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

                Mat raw = new Mat(sz.height, sz.width, CvType.CV_8UC1);
                raw.put(0, 0, data);
                Mat targ = Imgcodecs.imdecode(raw, 0);

                Log.i("Daniel", "sending picture to get AU's and prediction");
                int suggestion = A(targ.getNativeObjAddr());
                setSuggestions(Collections.singletonList(indexToEmoji(suggestion)), true, true);
                Log.i("Daniel", "got prediction");

                // SAVE IMAGE
//                Bitmap b = BitmapFactory.decodeByteArray(data, 0, data.length);
//                Utils.matToBitmap(targ,b);
//
////                sendText(String.valueOf(A(m3.getNativeObjAddr())));
//
//                File root = Environment.getExternalStorageDirectory();
//                String fname = "newnewnew" + ".jpg";
//                File file = new File(root + "/Download/", fname);
//                if (file.exists())
//                    file.delete();
//                try {
////                    android.os.Debug.waitForDebugger();
//                    FileOutputStream out = new FileOutputStream(file);
//                    b.compress(Bitmap.CompressFormat.JPEG, 90, out);
//                    out.flush();
//                    out.close();
//                } catch (Exception e) {
//                    e.printStackTrace();
//                }


                // NEW THREAD


//                new Thread(new Runnable() {
//                    @Override
//                    public void run() {
////                        int suggestion = A(m.getNativeObjAddr());
//                        if (suggestion != 0) {
////                            synchronized (result) {
////                                result.set(suggestion);
////                                updateCandidates();
////                            }
//                        }
//
//                        System.out.println("");
////                        synchronized (takePicture){
////                            takePicture.set(true);
////                        }
//
//                    }
//                }).start();


            }
        });
//                android.os.Debug.waitForDebugger();

        PermissionManager permissionManager = PermissionManager.getInstance(getApplicationContext());
        permissionManager.checkPermissions(Arrays.asList(Manifest.permission.CAMERA/*,Manifest.permission_group.CAMERA*/),
                new PermissionManager.PermissionRequestListener() {
                    @Override
                    public void onPermissionGranted() {
                        if (ContextCompat.checkSelfPermission(KeyMojiIME.this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
                            cameraView.setFacing(CameraView.FACING_FRONT);
                            cameraView.start();
                            Toast.makeText(getApplicationContext(), "Starting KeyMoji", Toast.LENGTH_LONG).show();
                            System.out.println("*********Permissions Granted");
                            Log.i("Daniel:************", "Permissions Granted");
                        }


                    }

                    @Override
                    public void onPermissionDenied() {
                        Toast.makeText(getApplicationContext(), "Permissions Denied*", Toast.LENGTH_SHORT).show();
                        System.out.println("*******Permissions Denied");
                        Log.i("Daniel:************", "Permissions Denied");
                    }
                });

        keyboardManager.switchToDefault(keyboardView);
        emojis = initializeEmojisMap();
        keyboardView.setOnKeyboardActionListener(this);


        // notify the user to allow access to the camera
        if (this.candidateView != null && !accessGranted)
            setSuggestions(Collections.singletonList(new Emoji(999, "authorize camera from notifications")), true, true);


        return mainLayout;
    }

    public Bitmap imageToBitmap() {
//        android.os.Debug.waitForDebugger();
        File root = Environment.getExternalStorageDirectory();
        Bitmap bMap = BitmapFactory.decodeFile(root + "/Download/fooImage1.jpg");
//        String fname = "fooImage1" + ".jpg";
//        File file = new File(root + "/Download/", fname);
//        if (file.exists())
//            file.delete();
//        try {
//            FileOutputStream out = new FileOutputStream(file);
//            bMap.compress(Bitmap.CompressFormat.JPEG, 90, out);
//            out.flush();
//            out.close();
//        } catch (Exception e) {
//            e.printStackTrace();
//        }
        return bMap;
    }

    public native int A(long add);

    private List<Emoji> initializeEmojisMap() {
        List<Emoji> map = new ArrayList<>();
        @SuppressLint("Recycle")
        TypedArray allUnicodes = getResources().obtainTypedArray(R.array.emojis_unicode);
        for (int i = 0; i < allUnicodes.length(); i++) {
            int[] unicodes = getResources().getIntArray(allUnicodes.getResourceId(i, 0));
            List<String> emojis = new ArrayList<>();
            for (int unicode : unicodes) {
                emojis.add(new String(Character.toChars(unicode)));
            }

            map.add(new Emoji(i + 1, emojis));

        }
        //int[] unicodes = getResources().getIntArray(R.array.emojis_unicode);

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

        if (!accessGranted)
            setSuggestions(Collections.singletonList(new Emoji(999, "authorize camera from notifications")), true, true);
//            this.candidateView.setPermissionMessage("Allow camera access in the notifications");

        return candidateView;
    }


    private void playClick(int primaryCode) {
        /*AudioManager am = (AudioManager) getSystemService(AUDIO_SERVICE);
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
        }*/
    }

    @Override
    public void onFinishInput() {
        super.onFinishInput();
        composing.setLength(0);
        updateCandidates();

//        // notify the user to allow access to the camera
//        if (this.candidateView != null && !accessGranted)
//            setSuggestions(Collections.singletonList(new Emoji(999,"authorize camera from notifications")), true, true);

        setCandidatesViewShown(false);
        if (cameraView != null && cameraView.isCameraOpened()) {
            disableCamera();
        }

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

    private void setSuggestions(List<Emoji> suggestions, boolean completions, boolean typedWordValid) {
        if ((suggestions != null && suggestions.size() > 0) || isExtractViewShown()) {
            setCandidatesViewShown(true);
        }
        this.suggestions = suggestions;
        if (this.candidateView != null) {

            this.candidateView.setSuggestions(suggestions, completions, typedWordValid, keyboardView.canvas);
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

    public void pickSuggestionManually(int emotion, int relativeIndex) {
        sendEmoji(indexToEmoji(emotion), relativeIndex);
    }

    enum ACTION {NOTHING}

    private Emoji indexToEmoji(int relativeIndex) {
        for (Emoji e : emojis) {
            if (e.getIndex() == relativeIndex) {
                return e;
            }
        }

        return emojis.get(0); // assume our emoji map is not empty
    }

    @Override
    public void onPress(int primaryCode) {
//        synchronized (takePicture){
//            takePicture.set(true);
//        }
//        cameraView.takePicture();
    }

    @Override
    public void onRelease(int primaryCode) {

    }


    private int charsToRemove(InputConnection ic) {
        CharSequence cs = ic.getTextBeforeCursor(2, 0);
        return isEmoji(cs) ? 2 : 1;
    }

    private static final int CHANGE_KEYBOARD = -9;
    @Override
    public void onKey(int primaryCode, int[] keyCodes) {
        InputConnection ic = getCurrentInputConnection();
        playClick(primaryCode);
        switch (primaryCode) {
            case Keyboard.KEYCODE_DELETE:
                ic.deleteSurroundingText(charsToRemove(ic), 0);
                break;
            case Keyboard.KEYCODE_SHIFT: {
                keyboardManager.switchKeyboardOnShift();
                keyboardView.invalidateAllKeys();
            }
            break;
            case Keyboard.KEYCODE_DONE:
                ic.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_ENTER));
                break;

            case CHANGE_KEYBOARD:
                keyboardManager.switchKeyboardOnChangeMode();

            case KeyMojiKeyboard.KEYCODE_MODE_CHANGE:
                ((InputMethodManager) getApplicationContext().getSystemService(INPUT_METHOD_SERVICE)).hideSoftInputFromInputMethod(
                        keyboardView.getWindowToken(), 0);
                break;
            case KeyMojiKeyboard.KEYCODE_LANGUAGE_SWITCH:
                ((InputMethodManager) getApplicationContext().getSystemService(INPUT_METHOD_SERVICE))
                        .showInputMethodPicker();
                break;


            default: {
                sendAsciiChar((char) primaryCode);
            }

        }

    }

    private void sendAsciiChar(char c) {
        sendText(String.valueOf(Character.isLetter(c) && !capsLock ?
                c : Character.toUpperCase(c)));
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

    class KeyboardManager {
        private Keyboard qwerty;
        private Keyboard symbols;
        private Keyboard numbers;
        private Keyboard active;
        private KeyboardView kv;

        KeyboardManager() {
        }

        void initKeyboards() {
            qwerty = new Keyboard(KeyMojiIME.this, R.xml.qwerty);
            numbers = new Keyboard(KeyMojiIME.this, R.xml.numbers);
            symbols = new Keyboard(KeyMojiIME.this, R.xml.symbols);
            active = qwerty;
        }

        private void switchKeyboard(boolean numbersShift) {
            if (numbersShift) {
                active = (active == numbers) ? symbols : numbers;
            } else {
                active = (active == qwerty) ? numbers : qwerty;
            }
            if (kv != null)
                kv.setKeyboard(active);

        }

        void switchKeyboardOnShift() {
            capsLock = !capsLock;
            if (active == numbers || active == symbols) {
                switchKeyboard(true);
            } else {
                qwerty.setShifted(capsLock);
            }

        }

        void switchKeyboardOnChangeMode() {
            switchKeyboard(false);
        }

        void switchToDefault(KeyboardView kv) {
            this.kv = kv;
            active = qwerty;
            if (kv != null)
                kv.setKeyboard(active);
        }
    }
}
