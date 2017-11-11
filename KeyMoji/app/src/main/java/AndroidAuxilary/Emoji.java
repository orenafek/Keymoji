package AndroidAuxilary;

import java.util.Collections;
import java.util.List;

/**
 * Created by danielohayon on 29/07/2017.
 */

public class Emoji {

    private int index;
    private List<String> emojiString;

    public Emoji(int index, List<String> emojiString) {
        this.index = index;
        this.emojiString = emojiString;
    }

    public Emoji(int index, String emojiString) {
        this.index = index;
        this.emojiString = Collections.singletonList(emojiString);
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public List<String> getEmojiStrings() {
        return emojiString;
    }

}
