package com.nc.pilot.lib;

import java.io.IOException;
import java.util.ArrayList;

/**
 * Created by travi on 2/5/2019.
 */
public class GcodeInterpreter {
    public class GcodeMove{
        public float Gword;
        public float Xword;
        public float Yword;
        public float Zword;
        public float Fword;
        public float Iword;
        public float Jword;
    }
    private ArrayList<GcodeMove> Moves = new ArrayList();

    //Active words
    private float Gword;
    private float Xword;
    private float Yword;
    private float Zword;
    private float Fword;
    private float Iword;
    private float Jword;

    public GcodeInterpreter(String file)
    {
        try {
            String buffer = GlobalData.readFile(file);
            String[] lines = buffer.split("\n");
            for (int x = 0; x < lines.length; x++)
            {
                parseLine(lines[x].toLowerCase());
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public ArrayList<GcodeMove> GetMoves()
    {
        return Moves;
    }
    public void checkWord(String line, char Word)
    {
        boolean capture = false;
        String word_builder = "";
        for (int x = 0; x < line.length(); x++)
        {
            if (line.charAt(x) == '(')
            {
                //Found comment
                break;
            }
            if (capture == true)
            {
                if (Character.isDigit(line.charAt(x)) || line.charAt(x) == '.' || line.charAt(x) == '-')
                {
                    word_builder = word_builder + line.charAt(x);
                }
                if ((Character.isAlphabetic(line.charAt(x)) && line.charAt(x) != ' ') || x == line.length() - 1)
                {
                    if (word_builder != "")
                    {
                        float word = new Float(word_builder);
                        if (Word == 'g')
                        {
                            Gword = word;
                        }
                        if (Word == 'x')
                        {
                            Xword = word;
                        }
                        if (Word == 'y')
                        {
                            Yword = word;
                        }
                        if (Word == 'z')
                        {
                            Zword = word;
                        }
                        if (Word == 'i')
                        {
                            Iword = word;
                        }
                        if (Word == 'j')
                        {
                            Jword = word;
                        }
                        if (Word == 'f')
                        {
                            Fword = word;
                        }
                    }
                    capture = false;
                    word_builder = "";
                }
            }
            if (line.charAt(x) == Word)
            {
                capture = true;
            }
        }
    }
    public void parseLine(String line)
    {
        //System.out.println("Line: " + line);
        checkWord(line, 'g');
        checkWord(line, 'x');
        checkWord(line, 'y');
        checkWord(line, 'z');
        checkWord(line, 'i');
        checkWord(line, 'j');
        checkWord(line, 'f');

        GcodeMove m = new GcodeMove();
        m.Gword = Gword;
        m.Xword = Xword;
        m.Yword = Yword;
        m.Zword = Zword;
        m.Iword = Iword;
        m.Jword = Jword;
        m.Fword = Fword;

        //Moves.add(m);
        if (Moves.size() > 0)
        {
            if (Moves.get(Moves.size() - 1).Xword != Xword || Moves.get(Moves.size() - 1).Yword != Yword || Moves.get(Moves.size() - 1).Zword != Zword || Moves.get(Moves.size() - 1).Iword != Iword || Moves.get(Moves.size() - 1).Jword != Jword)
            {
                if (Gword == 2 || Gword == 3)
                {
                    //System.out.println("G Entity-> G" + Gword + " X" + Xword + " Y" + Yword + " Z" + Zword + " I" + Iword + " J" + Jword + " F" + Fword);
                }
                else
                {
                    //System.out.println("G Entity-> G" + Gword + " X" + Xword + " Y" + Yword + " Z" + Zword + " F" + Fword);
                }
                Moves.add(m);
            }
        }
        else
        {
            Moves.add(m);
        }
    }
}
