package AndroidAuxilary;

/**
 * Created by oren.afek on 4/15/2017.
 */

public class Wrapper<T> {

    public T inner;
    private final T DEFAULT;

    public Wrapper(T value, T defaultValue) {
        this.inner = value;
        this.DEFAULT = defaultValue;
    }

    public Wrapper(T value) {
        this(value, value);
    }

    public T get() {
        return this.inner;
    }

    public Wrapper<T> setDefault() {
        return this.set(DEFAULT);
    }

    public Wrapper<T> set(T value) {
        this.inner = value;
        return this;
    }

    public boolean isDefaulted() {
        return this.get().equals(DEFAULT);
    }


}
