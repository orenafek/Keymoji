package AndroidAuxilary;

import android.inputmethodservice.InputMethodService;
import android.support.annotation.LayoutRes;
import android.view.LayoutInflater;

/**
 * @author oren
 * @since 27/02/17
 */

public class Inflater {

    LayoutInflater inflater;

    public Inflater(InputMethodService inputService) {
        this.inflater = inputService.getLayoutInflater();
    }

    public <T> T inflate(@LayoutRes int resourceId) {
        return (T) inflater.inflate(resourceId, null);
    }
}
