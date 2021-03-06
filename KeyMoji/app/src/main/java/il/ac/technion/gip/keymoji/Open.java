package il.ac.technion.gip.keymoji;

import android.Manifest;
import android.content.res.AssetManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.widget.TextView;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class Open extends AppCompatActivity {
    private static final String TAG = "OCVSample::Activity";
    private BaseLoaderCallback _baseLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS: {
                    Log.i(TAG, "OpenCV loaded successfully");
                }
                break;
                default: {
                    super.onManagerConnected(status);
                }
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_open);
        System.loadLibrary("native-lib");
        // Permissions for Android 6+
        ActivityCompat.requestPermissions(Open.this,
                new String[]{Manifest.permission.CAMERA, Manifest.permission.WRITE_EXTERNAL_STORAGE, Manifest.permission.READ_EXTERNAL_STORAGE},
                1);

        String toPath = "/data/data/" + getPackageName();  // Your application path
        copyAssetFolder(getAssets(), "", toPath);

        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_3_0_0, this, _baseLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            _baseLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
        TextView text = (TextView) findViewById(R.id.text);
//        text.setText();


    }


    public native String foo();


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
