package AndroidAuxilary;

import android.app.Activity;
import android.support.annotation.IdRes;
import android.support.v7.app.AppCompatActivity;

/**
 * @author oren
 * @since 07/03/17
 */

public class ViewAccessor {

    private Activity activity;

    public ViewAccessor(Activity activity){
        this.activity = activity;
    }
    public <T> T getView(@IdRes int id){
        return (T)activity.findViewById(id);
    }
}
