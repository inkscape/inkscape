import java.io.*;
import antlr.*;

public class Main {
  public static void main(String[] args) {
    try {
      IDLLexer lexer = new IDLLexer(new DataInputStream(System.in));
      IDLParser parser = new IDLParser(lexer);
      parser.specification();
    } catch(Exception e) {
      System.err.println("exception: "+e);
    }
  }
}

