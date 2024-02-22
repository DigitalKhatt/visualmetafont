using Microsoft.Office.Interop.Word;
using System;
using System.Collections.Generic;
using System.Diagnostics.Contracts;
using System.IO;
using System.Linq;
using System.Security.Principal;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace QuranTextGenerator.TarteelData
{
  public class Page
  {
    public int hizb { get; set; }
    public int juz { get; set; }
    public int pageNumber { get; set; }
    public int rub { get; set; }
    public Surah[] surahs { get; set; }
  }

  public class Surah
  {
    public int surahNum { get; set; }
    public Ayah[] ayahs { get; set; }
  }

  public class Ayah
  {
    public int ayahNum { get; set; }
    public Word[] words { get; set; }
  }

  public class Word
  {
    public string code { get; set; }
    public string text { get; set; }
    public string indopak { get; set; }
    public int lineNumber { get; set; }
    public int pageNumber { get; set; }
    public int surahNum { get; set; }
    public int ayahNum { get; set; }

  }


  internal static class TarteelData
  {
    public static List<Page> getPages()
    {
      List<Page> pages = new List<Page>();
      var root = @"files\tarteel\quran-assets\pages";

      var files = Directory.EnumerateFiles(root).OrderBy(a => a.Length).ThenBy(a => a);

      foreach (var file in files)
      {
        string jsonString = File.ReadAllText(file);

        Page page = JsonSerializer.Deserialize<Page>(jsonString);

        pages.Add(page);
      }

      return pages;
    }
    public static List<Word> getWords()
    {
      List<Word> words = new List<Word>();
      var pages = getPages();

      foreach (var page in pages)
      {
        foreach (var surah in page.surahs)
        {
          foreach (var ayah in surah.ayahs)
          {
            foreach (var word in ayah.words)
            {

              if (page.pageNumber == 262 && word.lineNumber == 8 && word.text == "مَا" && words.Last()?.text == "لَّوْ")
              {
                words.Last().text += "مَا";
              }
              else if (page.pageNumber == 378 && word.lineNumber == 12 && word.text == "لِىَ" && words.Last()?.text == "مَا")
              {
                words.Last().text += "لِىَ";
              }
              else
              {
                word.pageNumber = page.pageNumber;
                word.surahNum = surah.surahNum;
                word.ayahNum = ayah.ayahNum;

                words.Add(word);
              }

            }
          }
        }
      }

      return words;
    }

    public static List<List<string>> getLinesFromTarteel()
    {

      List<string> surabism = new List<string>();

      List<List<string>> pages = new List<List<string>>();

      string ver13 = Path.Combine(Directory.GetCurrentDirectory(), "files", "UthmanicHafs1 Ver13.doc");
      string ver13xml = Path.Combine(Directory.GetCurrentDirectory(), "xml", "UthmanicHafs1Ver13.xml");


      List<string> QPCWords = new List<string>();

      var qpcPages = QPCDoc.readXML(ver13xml); // QPCDoc.readPages(ver13);

      for (int pageNum = 0; pageNum < qpcPages.Count; pageNum++)
      {
        var page = qpcPages[pageNum];

        for (int lineNum = 0; lineNum < page.Count; lineNum++)
        {
          var line = page[lineNum];
          if (!line.StartsWith("سُورَةُ ") && !(line == "بِسۡمِ ٱللَّهِ ٱلرَّحۡمَٰنِ ٱلرَّحِيمِ" && pageNum != 0) && !(line == "بِّسۡمِ ٱللَّهِ ٱلرَّحۡمَٰنِ ٱلرَّحِيمِ"))
          {
            var lioneWords = line.Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries);
            QPCWords.AddRange(lioneWords);
          }
          else
          {
            surabism.Add(line);
          }
        }
      }

      var tarteelWords = getWords();

      if (tarteelWords.Count != QPCWords.Count)
      {
        throw new Exception("the number of words is different");
      }

      var lastPageNumber = 0;
      var lastLineNumber = 0;
      var surabasmIndex = 0;

      for (var wordIndex = 0; wordIndex < tarteelWords.Count; wordIndex++)
      {
        var tarteelWord = tarteelWords[wordIndex];
        var qpcWord = QPCWords[wordIndex];

        if (tarteelWord.pageNumber != lastPageNumber)
        {
          if (pages.Count > 2 && pages.Last().Count > 0)
          {
            while (lastLineNumber < 15)
            {
              pages.Last().Add(surabism[surabasmIndex++]);
              lastLineNumber++;
            }
          }
          pages.Add(new List<string>());
          lastPageNumber++;
          lastLineNumber = 0;
        }
        if (tarteelWord.lineNumber != lastLineNumber)
        {
          lastLineNumber++;
          while (tarteelWord.lineNumber != lastLineNumber)
          {
            pages.Last().Add(surabism[surabasmIndex++]);
            lastLineNumber++;
          }
          pages.Last().Add(qpcWord);
        }
        else
        {
          var page = pages.Last();
          page[page.Count - 1] += " " + qpcWord;

        }
      }

      /*
      Console.WriteLine($"QPC word counts={QPCWords.Count}");
      Console.WriteLine($"Tarteel word counts={tarteelWords.Count}");
      for (int i = 0; i < Math.Min(QPCWords.Count, tarteelWords.Count); i++)
      {
        if (QPCWords[i] != tarteelWords[i].text)
        {
          Console.WriteLine($"Page={tarteelWords[i].pageNumber},Line={tarteelWords[i].lineNumber},QPCWord={QPCWords[i]},TarteelWord={tarteelWords[i].text}");
        }
      }*/

      return pages;
    }
  }
}
