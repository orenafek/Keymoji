package il.ac.technion.gip.keymoji;

import android.content.Context;
import android.graphics.Canvas;
import android.inputmethodservice.KeyboardView;
import android.util.AttributeSet;

/**
 * Created by oren.afek on 5/6/2017.
 */

public class CustomizedKeyboardView extends KeyboardView {

    public Canvas canvas;

    public CustomizedKeyboardView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public CustomizedKeyboardView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }


    @Override
    public void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        this.canvas = canvas;
    }


}
