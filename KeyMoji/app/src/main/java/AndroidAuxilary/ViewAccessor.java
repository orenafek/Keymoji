package AndroidAuxilary;

import android.app.Activity;
import android.support.annotation.IdRes;
import android.view.View;

/**
 * @author oren
 * @since 07/03/17
 */

public class ViewAccessor {

    private Activity activity;
    private View view;

    public ViewAccessor(Activity activity){
        this(activity, null);
    }

    public ViewAccessor(View v) {
        this(null, v);
    }

    private ViewAccessor(Activity a, View v) {
        this.activity = a;
        this.view = v;
    }

    public <T> T getView(@IdRes int id){
        return activity != null ?
                (T) activity.findViewById(id) : (
                view != null ? (T) view.findViewById(id) : null);
    }
}
