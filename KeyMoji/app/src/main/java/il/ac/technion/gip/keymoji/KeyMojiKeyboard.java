package il.ac.technion.gip.keymoji;

import android.content.Context;
import android.inputmethodservice.Keyboard;

/**
 * Created by oren.afek on 4/25/2017.
 */

public class KeyMojiKeyboard extends Keyboard {

    static final int KEYCODE_LANGUAGE_SWITCH = -101;
    private Context context;

    public KeyMojiKeyboard(Context context, int xmlLayoutResId) {
        super(context, xmlLayoutResId);
        this.context = context;
        assertCodes();

    }

    private void assertCodes() {
        if (BuildConfig.DEBUG &&
                context.getResources().getInteger(R.integer.keycode_language_shift)
                        != KEYCODE_LANGUAGE_SWITCH) {

            throw new AssertionError();
        }
    }


}
