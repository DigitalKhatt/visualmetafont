using Microsoft.Office.Interop.Word;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection.PortableExecutable;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Xml;
namespace QuranTextGenerator
{
  internal static class QPCDoc
  {
    public static List<List<string>> readPages(string fileName)
    {
      Microsoft.Office.Interop.Word.Document docs = null;
      List<List<string>> Pages = new List<List<string>>();
      object miss = System.Reflection.Missing.Value;
      Microsoft.Office.Interop.Word.Application word = null;
      try
      {
        // create word application
        word = new Microsoft.Office.Interop.Word.Application();
        word.Visible = false;


        // create object of selected file path
        object path = fileName;
        // set file path mode
        object readOnly = true;
        // open document                
        docs = word.Documents.Open(ref path, ref miss, ref readOnly, ref miss, ref miss, ref miss, ref miss, ref miss, ref miss, ref miss, ref miss, ref miss, ref miss, ref miss, ref miss, ref miss);

        WdStatistic stat = WdStatistic.wdStatisticPages;
        int PagesCount = docs.ComputeStatistics(stat, ref miss);    // Get number of pages

        //Get pages
        object What = Microsoft.Office.Interop.Word.WdGoToItem.wdGoToPage;
        object Which = Microsoft.Office.Interop.Word.WdGoToDirection.wdGoToAbsolute;
        object Start;
        object End;
        object CurrentPageNumber;
        object NextPageNumber;

        for (int Index = 1; Index <= PagesCount; Index++)
        {
          /*
          CurrentPageNumber = Index; // (Convert.ToInt32(Index.ToString()));

          word.Selection.GoTo(ref What, ref Which, ref CurrentPageNumber, ref miss);
          string[] lines = docs.Bookmarks[@"\Page"].Range.Text.Replace("\r\f", "").Split(new char[] { Convert.ToChar(11), '\r' }, StringSplitOptions.RemoveEmptyEntries);
          List<string> newlines = new List<string>();
          for (int i = 0; i < lines.Length; i++)
          {
              string line = lines[i].Trim();
              if (line != "")
              {
                  newlines.Add(line);
              }
          }*/

          CurrentPageNumber = (Convert.ToInt32(Index.ToString()));
          NextPageNumber = (Convert.ToInt32((Index + 1).ToString()));

          // Get start position of current page
          Start = word.Selection.GoTo(ref What, ref Which, ref CurrentPageNumber, ref miss).Start;

          // Get end position of current page                                
          End = word.Selection.GoTo(ref What, ref Which, ref NextPageNumber, ref miss).End;

          // Get text
          string text = null;
          if (Convert.ToInt32(Start.ToString()) != Convert.ToInt32(End.ToString()))
            text = docs.Range(ref Start, ref End).Text;
          else
            text = docs.Range(ref Start).Text;

          string[] lines = text.Replace("\r\f", "").Split(new char[] { Convert.ToChar(11), '\r' }, StringSplitOptions.RemoveEmptyEntries);
          List<string> newlines = new List<string>();
          for (int i = 0; i < lines.Length; i++)
          {
            string line = lines[i].Trim();
            if (line != "")
            {
              newlines.Add(line);
            }
          }

          Pages.Add(newlines);
        }
      }
      finally
      {
        if (docs != null)
        {
          docs.Close(false);
        }

        if (word != null)
        {
          word.Quit(false);
        }
        docs = null;
        word = null;
        GC.Collect();
      }

      return Pages;
    }

    public static List<List<string>> readXML(string fileName)
    {
      List<List<string>> pages = new List<List<string>>();
      List<string> lines = null;

      using (XmlReader reader = XmlReader.Create(fileName))
      {
        while (reader.Read())
        {
          if ((reader.NodeType == XmlNodeType.Element) && (reader.Name == "Page"))
          {
            lines = new List<string>();
          }
          else if ((reader.NodeType == XmlNodeType.Element) && (reader.Name == "Line"))
          {
            lines.Add(reader.ReadElementContentAsString());
          }
          else if ((reader.NodeType == XmlNodeType.EndElement) && (reader.Name == "Page"))
          {
            pages.Add(lines);
          }
        }
      }

      return pages;
    }

    public static void GenerateCPPFromUthmanicHafs(bool quranComplex, string fileName)
    {
      List<List<string>> UthmanicHafsDocPages = QPCDoc.readXML(fileName);

      QPCDoc.GenerateCPPFromUthmanicHafs(quranComplex, UthmanicHafsDocPages);
    }

    public static void GenerateCPPFromUthmanicHafs(bool quranComplex, List<List<string>> UthmanicHafsDocPages)
    {
      string outputfile = @"output/quran.cpp";
      if (quranComplex)
      {
        outputfile = @"D:output/qurancomplex.cpp";
      }
      //string suraWord = "سُورَةُ";
      //string bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"; // "بِسْمِ ٱللَّهِ ٱلرَّحْمَـٰنِ ٱلرَّحِيمِ";
      using (System.IO.StreamWriter file = new System.IO.StreamWriter(outputfile))
      {
        if (quranComplex)
        {
          file.WriteLine("char const * quranComplex[604] = {");
        }
        else
        {
          file.WriteLine("char const * qurantext[604] = {");
        }

        for (int nopage = 0; nopage < UthmanicHafsDocPages.Count; nopage++)
        {
          file.WriteLine("R\"page" + nopage.ToString() + "(");
          var page = UthmanicHafsDocPages[nopage];
          for (int i = 0; i < page.Count; i++)
          {

            string line = page[i];
            if (!quranComplex)
            {
              line = replaceCharcatersFromUthmanicHafsDoc(page[i], nopage + 1, i + 1);
            }
            else
            {
              line = line.Replace("\u06E4", ""); // Small Madda for Sajda Rule above used to add line above. We dont need this.
            }


            file.WriteLine(line);

          }

          if (nopage != 603)
          {
            file.WriteLine(")page" + nopage.ToString() + "\",");
          }
          else
          {
            file.WriteLine(")page" + nopage.ToString() + "\"");
          }
        }
        file.WriteLine("};");


      }
    }

    public static string replaceCharcatersFromUthmanicHafsDoc(string line, int pageNumber, int lineNumber)
    {
      // 0652 : ARABIC SUKUN ->  06DF : ARABIC SMALL HIGH ROUNDED ZERO
      string newline = line.Replace("\u0652", "\u06DF");

      // 06E! : ARABIC SMALL HIGH DOTLESS HEAD OF KHAH -> 0652 : ARABIC SUKUN
      newline = newline.Replace("\u06E1", "\u0652");

      // -> 08F0 ARABIC OPEN FATHATAN
      newline = newline.Replace("\u0657", "\u08F0");

      // -> 08F1 ARABIC OPEN DAMMATAN
      newline = newline.Replace("\u065E", "\u08F1");

      // -> 08F2 ARABIC OPEN KASRATAN
      newline = newline.Replace("\u0656", "\u08F2");

      //Already done
      //newline = newline.Replace("\u06E4", ""); // Small Madda for Sajda Rule above 

      // miss sukun in فَٱدَّٰرَٰٔتُمۡ page 11 line 5
      newline = newline.Replace("\u064E\u0670\u0654\u062A", "\u064E\u0670\u034F\u0654\u0652\u062A");

      // suppress tatweel in لِّـجِبۡرِيلَ page 15 line 7
      newline = newline.Replace("\u0650\u0640\u062C\u0650\u0628", "\u0650\u062C\u0650\u0628");

      // miss letter in لِيَسُـۥٓـُٔوا۟ page 282 line 14
      newline = newline.Replace("\u0633\u064F\u0640\u0654\u064F", "\u0633\u064F\u034F\u08F3\u0653\u0640\u0654\u064F");

      // 0648  ARABIC LETTER WAW + 0655  ARABIC HAMZA BELOW -> 0624  ARABIC LETTER WAW WITH HAMZA ABOVE
      newline = newline.Replace("\u0648\u0655", "\u0624");

      //ARABIC LETTER YEH WITH HAMZA ABOVE
      newline = newline.Replace("\u064A\u0655", "\u0626");

      //ARABIC LETTER YEH WITH HAMZA ABOVE foloweed by /0640  ZERO WIDTH JOINER only in "سُورَةُ المَائ‍ِدَةِ"
      newline = newline.Replace("\u0626\u0640", "\u0626");

      // ARABIC SMALL HIGH SEEN + FATHA -> ARABIC SMALL LOW SEEN + FATHA
      if (newline.Contains("\u06DC\u064E"))
      {
        Console.WriteLine($"ARABIC SMALL HIGH SEEN + FATHA at {pageNumber}-{lineNumber}");
      }
      if (newline.Contains("\u06E3\u064E"))
      {
        Console.WriteLine($"ARABIC SMALL LOW SEEN + FATHA at {pageNumber}-{lineNumber}");
      }
      newline = newline.Replace("\u06DC\u064E", "\u06E3\u064E");

      // ALEF + HAMZA ABOVE + MADDAH ABOVE
      if (newline.Contains("\u0627\u0654\u0653"))
      {
        Console.WriteLine($"Alef + hamza above + madda above at {pageNumber}-{lineNumber}");
      }
      // ALEF WITH HAMZA ABOVE + MADDAH ABOVE
      if (newline.Contains("\u0623\u0653"))
      {
        //Console.WriteLine($"ALEF WITH HAMZA ABOVE + MADDAH ABOVE at {pageNumber}-{lineNumber}");
        newline = newline.Replace("\u0623\u0653", "\u0640\u0654\u064E\u0627");
      }



      //Dont reorder  marks        
      newline = newline.Replace("\u0652\u06DC", "\u0652\u034F\u06DC");

      // change space position after hizb symbol
      string pattern = "\\s*" + "۞" + "\\s*";
      newline = Regex.Replace(newline, pattern, "۞" + " ");

      // Small Madda for Sajda Rule above used to add line above. We dont need this.
      newline = newline.Replace("\u06E4", "");

      //Add ۝ (END OF AYAH 0x06DD) before digits
      Regex regex = new Regex("(\\d+)");
      newline = regex.Replace(newline, "۝" + "$1");

      //decomposition

      newline = newline.Replace("\u0622", "\u0627\u0653"); // ARABIC LETTER ALEF WITH MADDA ABOVE
      newline = newline.Replace("\u0623", "\u0627\u0654"); // ARABIC LETTER ALEF WITH HAMZA ABOVE
      newline = newline.Replace("\u0624", "\u0648\u0654"); // ARABIC LETTER WAW WITH HAMZA ABOVE
      newline = newline.Replace("\u0625", "\u0627\u0655"); // ARABIC LETTER ALEF WITH HAMZA BELOW
      newline = newline.Replace("\u0626", "\u064A\u0654"); // ARABIC LETTER YEH WITH HAMZA ABOVE

      // Avoid normalization to match text <-> glyph unambiguously in order to correctly apply features and styles
      newline = newline.Replace("\u0627\u0653", "\u0627\u034F\u0653");
      newline = newline.Replace("\u0627\u0654", "\u0627\u034F\u0654\u034F");
      newline = newline.Replace("\u0648\u0654", "\u0648\u034F\u0654\u034F");      
      newline = newline.Replace("\u064A\u0654", "\u064A\u034F\u0654\u034F");

      return newline;

    }

  }
}
