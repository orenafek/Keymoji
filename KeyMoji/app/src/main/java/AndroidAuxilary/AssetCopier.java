package AndroidAuxilary;

import android.content.res.AssetManager;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by oren.afek on 4/15/2017.
 */

public class AssetCopier {

    public static boolean copyAssetFolder(AssetManager assetManager,
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
