package AndroidAuxilary;

/**
 * Created by danielohayon on 29/07/2017.
 */

public class Emoji {

    private int index;
    private String emojiString;

    public Emoji(int index, String emojiString) {
        this.index = index;
        this.emojiString = emojiString;
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public String getEmojiString() {
        return emojiString;
    }

    public void setEmojiString(String emojiString) {
        this.emojiString = emojiString;
    }
}
