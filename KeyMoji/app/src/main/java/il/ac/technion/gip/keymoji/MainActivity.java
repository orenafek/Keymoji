package il.ac.technion.gip.keymoji;

import android.Manifest;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.util.SparseArray;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import AndroidAuxilary.ViewAccessor;

public class MainActivity extends AppCompatActivity implements CameraBridgeViewBase.CvCameraViewListener2 {

    private static final String TAG = "OCVSample::Activity";
    private JavaCameraView _cameraBridgeViewBase;
    private int frameCounter = 0;
    private ViewAccessor viewAccessor = new ViewAccessor(this);
    private int result;
    private TextView main_debug_tv;
    private static SparseArray<String> emotionsMap;

    private BaseLoaderCallback _baseLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i(TAG, "OpenCV loaded successfully");
                    _cameraBridgeViewBase.enableView();
                }
                break;
                default: {
                    super.onManagerConnected(status);
                }
            }
        }
    };

    static {
        emotionsMap = new SparseArray<>(6);
        emotionsMap.put(0, "Unknown");
        emotionsMap.put(1, "Anger");
        emotionsMap.put(2, "Disgust");
        emotionsMap.put(3, "Fear");
        emotionsMap.put(4, "Happy");
        emotionsMap.put(5, "Sad");
        emotionsMap.put(6, "Surprised");
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);


        // Permissions for Android 6+
        ActivityCompat.requestPermissions(MainActivity.this,
                new String[]{Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE},
                1);

        String toPath = "/data/data/" + getPackageName();  // Your application path
        copyAssetFolder(getAssets(), "", toPath);


        // Load ndk built module, as specified
        // in moduleName in build.gradle
        System.loadLibrary("native-lib");

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_main);


        _cameraBridgeViewBase = (JavaCameraView) findViewById(R.id.main_surface);
        _cameraBridgeViewBase.setVisibility(SurfaceView.VISIBLE);
        _cameraBridgeViewBase.setCvCameraViewListener(this);


        Button b = viewAccessor.getView(R.id.main_btn_show_result);
        b.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                appendResult();
            }
        });


    }

    @Override
    public void onPause() {
        super.onPause();
        disableCamera();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, _baseLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            _baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[], int[] grantResults) {
        switch (requestCode) {
            case 1: {
                // If request is cancelled, the result arrays are empty.
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // permission was granted, yay! Do the
                    // contacts-related task you need to do.
                } else {
                    // permission denied, boo! Disable the
                    // functionality that depends on this permission.
                    Toast.makeText(MainActivity.this, "Permission denied to read your External storage", Toast.LENGTH_SHORT).show();
                }
                return;
            }
            // other 'case' lines to check for other
            // permissions this app might request
        }
    }

    public void onDestroy() {
        super.onDestroy();
        disableCamera();
    }

    private int occurrencesOf(CharSequence s, char c) {
        int result = 0;
        for (int i = 0; i < s.length(); ++i) {
            char ch = s.charAt(i);
            result += ch == c ? 1 : 0;
        }

        return result;
    }

    private void appendResult() {
        main_debug_tv = viewAccessor.getView(R.id.main_debug_tv);
        if (main_debug_tv != null) {
            if (occurrencesOf(main_debug_tv.getText(), '\n') > 6) {
                main_debug_tv.setText("");
            }
            main_debug_tv.setText(main_debug_tv.getText() + "\n" + emotionsMap.get(result));
        }
    }

    public void disableCamera() {
        if (_cameraBridgeViewBase != null)
            _cameraBridgeViewBase.disableView();
    }

    public void onCameraViewStarted(int width, int height) {
    }

    public void onCameraViewStopped() {
    }

    public Mat onCameraFrame(CameraBridgeViewBase.CvCameraViewFrame inputFrame) {
        Mat matGray = inputFrame.gray();
        if (frameCounter % 30 == 0) {
            //TODO: Flip image
//        Core.flip(matGray.t(), matGray, 1);
            result = getEmotion(matGray.getNativeObjAddr());
        }
        frameCounter++;
        return matGray;
    }

    public native int getEmotion(long matAddrGray);

    private static boolean copyAssetFolder(AssetManager assetManager,
                                           String fromAssetPath, String toPath) {
        try {
            String[] files = assetManager.list(fromAssetPath);
            if (!fromAssetPath.equals("")) {
                //Not root path
                fromAssetPath = fromAssetPath + "/";
            }
            new File(toPath).mkdirs();
            boolean res = true;
            for (String file : files) {
                if (file.contains(".")) {
                    Log.d("OPENCV_TAG", "Writing" + file);
                    res &= copyAsset(assetManager,
                            fromAssetPath + file,
                            toPath + "/" + file);
                } else {
                    res &= copyAssetFolder(assetManager,
                            fromAssetPath + file,
                            toPath + "/" + file);
                }
            }
            return res;
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    private static boolean copyAsset(AssetManager assetManager,
                                     String fromAssetPath, String toPath) {
        InputStream in = null;
        OutputStream out = null;
        try {
            in = assetManager.open(fromAssetPath);
            new File(toPath).createNewFile();
            out = new FileOutputStream(toPath);
            copyFile(in, out);
            in.close();
            in = null;
            out.flush();
            out.close();
            out = null;
            Log.d("OPENCV_TAG", "Yay!");
            return true;
        } catch (Exception e) {
            Log.d("OPENCV_TAG", "Bummer :(");
            e.printStackTrace();
            return false;
        }
    }

    private static void copyFile(InputStream in, OutputStream out) throws IOException {
        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }
    }

}

