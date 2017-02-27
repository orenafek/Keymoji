package ch.hepia.iti.opencvnativeandroidstudio;

import android.inputmethodservice.InputMethodService;
import android.inputmethodservice.Keyboard;
import android.inputmethodservice.KeyboardView;
import android.media.AudioManager;
import android.view.KeyEvent;
import android.view.View;
import android.view.inputmethod.InputConnection;

import AndroidAuxilary.Inflater;

/**
 * @author Oren Afek
 * @since 27/02/17
 */

public class KeyMojiIME extends InputMethodService {

    private KeyboardView keyboardView;
    private Keyboard keyboard;

    private boolean capsLock = false;

    @Override
    public View onCreateInputView() {
        keyboardView = new Inflater(this).inflate(R.layout.keyboard);
        keyboard = new Keyboard(this, R.xml.qwerty);
        keyboardView.setKeyboard(keyboard);
        keyboardView.setOnKeyboardActionListener(new KeyboardView.OnKeyboardActionListener() {
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
                    case Keyboard.KEYCODE_SHIFT:
                        capsLock = !capsLock;
                        keyboard.setShifted(capsLock);
                        keyboardView.invalidateAllKeys();
                        break;
                    case Keyboard.KEYCODE_DONE:
                        ic.sendKeyEvent(new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_ENTER));
                        break;
                    default:
                        char c = (char) primaryCode;
                        ic.commitText(String.valueOf(Character.isLetter(c) && capsLock ?
                                c : Character.toUpperCase(c)), 1);


                }
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
        });
        return keyboardView;
    }

    private void playClick(int primaryCode) {
        AudioManager am = (AudioManager) getSystemService(AUDIO_SERVICE);
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
}
