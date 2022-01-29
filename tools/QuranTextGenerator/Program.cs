using System;
using System.Collections.Generic;
using System.Linq;
using System.Xml;
using System.Threading;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json;
using System.IO;
using System.Dynamic;
using System.Xml.Linq;
using System.Windows.Forms;
using Microsoft.Office.Interop.Word;
using System.Text;
using System.Text.RegularExpressions;
using DiffMatchPatch;
using System.Globalization;

namespace GenerateTexFromTanzil
{

    enum DiffType
    {
        REPLACMENT,
        ADD,
        DELETE
    }
    struct LineInfo
    {
        public int pageIndex { get; set; }
        public int lineIndex { get; set; }
        public string text { get; set; }
        public int position { get; set; }
    }

    struct DiffInfo
    {
        public LineInfo source;
        public LineInfo dest;
    }

    static class Program
    {

        static Dictionary<int, decimal> lineWidths = new Dictionary<int, decimal>
        {
            { 601 * 3, 1 },
            { 601 * 4, 1 },
            { 601 * 7, 1 },
            { 601 * 8, 1 },
            { 601 * 9, 1 },
            { 601 * 10, 1 },
            { 601 * 13, 1 },
            { 601 * 14, 1 },
            { 601 * 15, 1 },
            { 602 * 5, 0.63M },
            { 602 * 11, 0.9M },
            { 602 * 15, 0.53M },
            { 603 * 10, 0.66M },
            { 603 * 13, 1 },
            { 603 * 15, 0.66M },
            { 604 * 3, 1 },
            { 604 * 4, 0.55M },
            { 604 * 7, 1 },
            { 604 * 8, 1 },
            { 604 * 9, 0.55M },
            { 604 * 12, 1 },
            { 604 * 13, 1 },
            { 604 * 14, 0.675M },
            { 604 * 15, 0.5M },
        };

        static Dictionary<int, decimal> madinaLineWidths = new Dictionary<int, decimal>
        {
           { 586 * 1, 0.81M},
           { 593 * 2, 0.81M},
           { 594 * 5, 0.63M},
        };
        static decimal getWidth(int pageNo, int lineNo, bool madinaFormat)
        {
            pageNo++;
            lineNo++;
            decimal width = 0;
            if (lineWidths.TryGetValue(pageNo * lineNo, out width))
            {
                return width;
            }
            else
            {
                if (madinaFormat && madinaLineWidths.TryGetValue(pageNo * lineNo, out width))
                {
                    return width;
                }
                else
                {
                    return width;
                }
            }
        }
        static void Main(string[] args)
        {
            Console.OutputEncoding = Encoding.UTF8;

            string arg = "";
            if (args.Length > 0)
            {
                arg = args[0];
            }

            string fileName = "xml\\UthmanicHafs1Ver13.xml";


            if (arg == "xml")
            {
                string currentDir = Directory.GetCurrentDirectory();

                string ver9 = currentDir + "\\files\\UthmanicHafs1 Ver09.doc";
                string ver13 = currentDir + "\\files\\UthmanicHafs1 Ver13.doc";

                GenerateXMLfromUthmanicHafsDoc(ver9, @"xml\\UthmanicHafs1Ver09.xml");
                GenerateXMLfromUthmanicHafsDoc(ver13, @"xml\\UthmanicHafs1Ver13.xml");
            }
            else if (arg == "compare")
            {
                string currentDir = Directory.GetCurrentDirectory();

                string ver16 = currentDir + "\\files\\UthmanicHafs1 Ver16.doc";
                string ver13 = currentDir + "\\files\\UthmanicHafs1 Ver13.doc";

                compareHafsDocs(ver13, ver16);
            }
            else if (arg == "cpp")
            {
                GenerateCPPFromUthmanicHafs(false, fileName);
                GenerateCPPFromUthmanicHafs(true, fileName);
            }
            else if (arg == "tex")
            {
                GenerateContextFileFromDocFile(fileName, "par");
            }
            else
            {

                GenerateCPPFromUthmanicHafs(false, fileName);
                GenerateCPPFromUthmanicHafs(true, fileName);
                GenerateContextFileFromDocFile(fileName, "par");
                GenerateContextFileFromDocFile(fileName, "exact");
                GenerateContextFileFromDocFile(fileName, "page");
                GenerateLatexFileFromDocFile(fileName, false);
                GenerateLatexFileFromDocFile(fileName, true);

            }

        }

        static void getText(string fileName, out Dictionary<int, LineInfo> info, out string text)
        {
            var pages = readPages(fileName);

            info = new Dictionary<int, LineInfo>();
            text = "";
            int position = 0;
            for (int pageIndex = 0; pageIndex < pages.Count; pageIndex++)
            {
                var page = pages[pageIndex];

                for (int lineIndex = 0; lineIndex < page.Count; lineIndex++)
                {
                    var line = page[lineIndex];

                    LineInfo lineinfo = new LineInfo { lineIndex = lineIndex, pageIndex = pageIndex, text = line };
                    info.Add(position, lineinfo);
                    if (text == "")
                    {
                        text = line;
                    }
                    else
                    {
                        text += " " + line;
                    }
                    position += line.Length + 1;
                }
            }
        }
        static void compareHafsDocs(string fileName1, string fileName2)
        {

            Dictionary<int, LineInfo> info1;
            Dictionary<int, LineInfo> info2;
            string text1;
            string text2;

            getText(fileName1, out info1, out text1);
            getText(fileName2, out info2, out text2);

            //var text1 = string.Join(" ", pages1.Select(a => string.Join(" ", a.ToArray())).ToArray());
            //var text2 = string.Join(" ", pages2.Select(a => string.Join(" ", a.ToArray())).ToArray());

            //text1 = text1.Replace('\u200D', '\u0640');
            //text1 = text1.Replace('\u06E2', '\u06ED');
            //text2 = text2.Replace('\u06E2', '\u06ED');

            diff_match_patch dmp = new diff_match_patch();
            dmp.Diff_Timeout = 0;

            // Execute one reverse diff as a warmup.
            var diffs = dmp.diff_main(text1, text2);

            Dictionary<string, List<DiffInfo>> result = new Dictionary<string, List<DiffInfo>>();

            int position1 = 0;
            int position2 = 0;

            for (int i = 0; i < diffs.Count; i++)
            {
                var diff = diffs[i];

                if (diff.operation != Operation.EQUAL)
                {
                    string output = "";
                    int prevposition1 = position1;
                    int prevposition2 = position2;
                    foreach (char c in diff.text)
                    {
                        output += "U" + ((int)c).ToString("X4");
                    }
                    if (diff.operation == Operation.DELETE)
                    {

                        int j = i + 1;
                        if (j < diffs.Count && diffs[j].operation == Operation.INSERT)
                        {
                            string insert = "";
                            foreach (char c in diffs[j].text)
                            {
                                insert += "U" + ((int)c).ToString("X4");
                            }
                            output = "-" + output + "+" + insert;
                            i++;
                            position2 += diffs[j].text.Length;
                        }
                        else
                        {
                            output = "-" + output;
                        }
                        position1 += diff.text.Length;
                    }
                    else
                    {
                        output = "+" + output;
                        position2 += diff.text.Length;
                    }

                    var sourcekey = info1.LastOrDefault(num => num.Key <= prevposition1);
                    var source = sourcekey.Value;
                    source.position = prevposition1 - sourcekey.Key;

                    var destKey = info2.LastOrDefault(num => num.Key <= prevposition2);
                    var dest = destKey.Value;
                    dest.position = prevposition2 - destKey.Key;

                    DiffInfo diffinfo = new DiffInfo { source = source, dest = dest };

                    List<DiffInfo> list = null;

                    if (!result.TryGetValue(output, out list))
                    {
                        list = new List<DiffInfo>();
                        result.Add(output, list);
                    }

                    list.Add(diffinfo);
                }
                else
                {
                    position1 += diff.text.Length;
                    position2 += diff.text.Length;
                }

            }

            foreach (var change in result)
            {
                Console.WriteLine(change.Key);
                foreach (var diff in change.Value)
                {
                    string source = "source=" + (diff.source.pageIndex + 1) + "-" + (diff.source.lineIndex + 1) + "-" + (diff.source.position + 1);
                    string dest = "dest=" + (diff.dest.pageIndex + 1) + "-" + (diff.dest.lineIndex + 1) + "-" + (diff.dest.position + 1);

                    Console.WriteLine("\t" + source + "," + dest);
                }
            }

        }
        static void compareHafsDocsPageByPage(string fileName1, string fileName2)
        {

            var pages1 = readPages(fileName1);
            var pages2 = readPages(fileName2);

            for (int i = 0; i < 604; i++)
            {
                string text1 = string.Join(" ", pages1[i].ToArray());
                string text2 = string.Join(" ", pages2[i].ToArray());

                text1 = text1.Replace('\u200D', '\u0640');
                //text1 = text1.Replace('\u06E2', '\u06ED');

                diff_match_patch dmp = new diff_match_patch();
                dmp.Diff_Timeout = 0;

                // Execute one reverse diff as a warmup.
                var diffs = dmp.diff_main(text1, text2);
                string output = "";
                if (diffs != null)
                {
                    foreach (var diff in diffs)
                    {
                        if (diff.operation != Operation.EQUAL)
                        {
                            if (diff.operation == Operation.DELETE)
                            {
                                output += ",DELETE(";
                            }
                            else
                            {
                                output += ",INSERT(";
                            }

                            foreach (char c in diff.text)
                            {
                                output += ((int)c).ToString("X4");
                            }

                            output += ")";
                        }


                    }
                }
                if (output != "")
                {
                    Console.WriteLine("Page " + i + output);
                }
                GC.Collect();
                GC.WaitForPendingFinalizers();
                /*
                DateTime ms_start = DateTime.Now;
                dmp.diff_main(text1, text2);
                DateTime ms_end = DateTime.Now;

                Console.WriteLine("Elapsed time: " + (ms_end - ms_start));*/
            }

        }
        static List<List<string>> readXML(string fileName)
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
        static void GenerateCPPFromUthmanicHafs(bool quranComplex, string fileName)
        {

            List<List<string>> UthmanicHafsDocPages = readXML(fileName);

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
                            line = replaceCharcatersFromUthmanicHafsDoc(page[i]);
                        }
                        else
                        {
                            line = line.Replace("\u06E4", ""); // Small Madda for Sajda Rule above used to add line above. We dont need this.
                        }
                        /*
                        if (line.Contains("\u06E4"))
                        {
                            Console.WriteLine("Small Madda for Sajda Rule above, page=" + (nopage + 1).ToString() + ",line=" + (i + 1).ToString());
                        }*/



                        //int indexbism = line.IndexOf(bism);

                        /* v09 only
                        if ((nopage + 1) == 567 && (i + 1) == 12)
                        {
                            line = line.Replace("٢٩", "").Trim();

                        }
                        else if ((nopage + 1) == 567 && (i + 1) == 13)
                        {
                            line = "٢٩" + " " + line;
                        }*/

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
        static void GenerateLatexFileFromDocFile(string fileName, bool madinaFormat)
        {
            List<List<string>> UthmanicHafsDocPages = readXML(fileName);
            string outputfile = @"output/quran_text" + ".tex";

            if (madinaFormat)
            {
                outputfile = @"output/quran_text_madina" + ".tex";
            }

            string bismPattern = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ" + "|" + "بِّسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ";
            string sajdapatterns = @"(وَٱسْجُدْ) وَٱقْتَرِب|(خَرُّوا۟ سُجَّدࣰا)|(وَلِلَّهِ يَسْجُدُ)|(يَسْجُدُونَ)۩|(فَٱسْجُدُوا۟ لِلَّهِ)|(وَٱسْجُدُوا۟ لِلَّهِ)|(أَلَّا يَسْجُدُوا۟ لِلَّهِ)|(وَخَرَّ رَاكِعࣰا)|(يَسْجُدُ لَهُ)|(يَخِرُّونَ لِلْأَذْقَانِ سُجَّدࣰا)|(ٱسْجُدُوا۟) لِلرَّحْمَٰنِ|ٱرْكَعُوا۟ (وَٱسْجُدُوا۟)"; // sajdapatterns.replace("\u0657", "\u08F0").replace("\u065E", "\u08F1").replace("\u0656", "\u08F2");

            using (System.IO.StreamWriter file = new System.IO.StreamWriter(outputfile))
            {
                for (int nopage = 0; nopage < UthmanicHafsDocPages.Count; nopage++)
                {

                    if (nopage < 581) continue;

                    //if (nopage > 1 && nopage < 550) continue;

                    var page = UthmanicHafsDocPages[nopage];
                    if (nopage == 0 || nopage == 1)
                    {
                        file.Write("\\topglue0.1\\pageheight\n"); //%\topglue 0pt plus 1fill
                        file.Write("{\\fatiha\\textdir TRT\n"); //%\topglue 0pt plus 1fill
                    }
                    for (int i = 0; i < page.Count; i++)
                    {
                        var mm = 45;
                        if (i == 1 || i == 7)
                        {
                            mm = 45;
                        }
                        else if (i == 2 || i == 6)
                        {
                            mm = 65;
                        }
                        else if (i == 3 || i == 5)
                        {
                            mm = 85;
                        }
                        else
                        {
                            mm = 9999;
                        }

                        string line = replaceCharcatersFromUthmanicHafsDoc(page[i]);
                        if (nopage == 0 || nopage == 1)
                        {
                            if (line.StartsWith("سُورَةُ "))
                            {
                                file.WriteLine(@"\leavevmode\suraline{" + line + @"}\vskip0.1\pageheight");
                            }
                            else
                            {
                                file.WriteLine(@"\centerline{\hbox to0." + mm + @"\textwidth{" + line + @"}}");
                            }

                        }
                        else
                        {
                            if (line.StartsWith("سُورَةُ "))
                            {
                                file.WriteLine(@"\suraline{" + line + @"}");
                            }
                            else
                            {
                                var match = Regex.Match(line.Trim(), bismPattern);
                                if (match.Success)
                                {
                                    file.WriteLine(@"\bismline{" + match.Value + @"}");
                                }
                                else
                                {

                                    line = Regex.Replace(line.Trim(), sajdapatterns, delegate (Match m)
                                    {
                                        return "\\sajdabar{" + m.Value + "}";
                                    });

                                    decimal width = getWidth(nopage, i, madinaFormat);

                                    if (width == 0)
                                    {
                                        if (madinaFormat)
                                        {
                                            file.WriteLine(@"\hbox to\textwidth{" + line + @"}");
                                        }
                                        else
                                        {
                                            file.WriteLine(line);
                                        }

                                    }
                                    else
                                    {
                                        file.WriteLine(@"\centerline{\hbox to" + width + @"\textwidth{" + line + @"}}");
                                    }


                                }
                            }


                        }
                    }
                    if (nopage == 0 || nopage == 1)
                    {
                        file.Write("}\n"); //{\\fatiha\\textdir TRT\n
                        file.Write(@"\vfill
\newpage
");
                    }
                    else if (nopage == 602 || nopage == 601 || nopage == 600 || nopage == 599)
                    {
                        file.Write("\\newpage\n");
                    }
                    else if (madinaFormat)
                    {
                        file.Write("\\newpage\n");
                    }
                }
            }
        }
        static void GenerateContextFileFromDocFile(string fileName, string arg)
        {
            List<List<string>> UthmanicHafsDocPages = readXML(fileName);

            string outputfile = @"output /quran_context_" + arg + ".tex";
            string suraWord = "سُورَةُ";
            string bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَٰنِ ٱلرَّحِيمِ"; // "بِسْمِ ٱللَّهِ ٱلرَّحْمَـٰنِ ٱلرَّحِيمِ";
            string bism2 = "بِسۡمِ ٱللَّهِ ٱلرَّحۡمَٰنِ ٱلرَّحِيمِ";
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(outputfile))
            {


                file.WriteLine(@"\environment ara-sty");
                file.WriteLine(@"\setarabic");
                file.WriteLine(@"\starttext");
                file.WriteLine(@"\testnaksh");
                file.WriteLine();
                file.WriteLine(@"\baselineskip =9.44mm plus 0pt minus 0pt");
                file.WriteLine(@"\lineskip=4.2pt plus 0pt minus 0pt");
                file.WriteLine(@"\lineskiplimit=-100pt");
                file.WriteLine(@"\hbadness=10000");
                file.WriteLine(@"\clubpenalty=0%");
                file.WriteLine(@"\widowpenalty=0%");
                file.WriteLine(@"\displaywidowpenalty=0%");
                file.WriteLine(@"\brokenpenalty=0%");
                file.WriteLine();

                int currentpage = 1;


                for (int nopage = 0; nopage < UthmanicHafsDocPages.Count; nopage++)
                {
                    var page = UthmanicHafsDocPages[nopage];

                    if (nopage == 0 || nopage == 1)
                    {
                        file.WriteLine(@"\leavevmode\vfill");
                    }

                    for (int i = 0; i < page.Count; i++)
                    {
                        string line = page[i]; // replaceCharcatersFromUthmanicHafsDoc(page[i]);
                        line = setSajdahBar(line);
                        //int indexbism = line.IndexOf(bism);
                        if (line.StartsWith("سُورَةُ "))
                        {
                            file.WriteLine(@"\sura{" + line + @"}");
                            //file.WriteLine(@"\startlinealignment[middle]");
                            //file.WriteLine(@"\ArabicGlobalDir ");
                            //file.WriteLine(line);
                            //file.WriteLine(@"\stoplinealignment\par");
                        }
                        //else if (indexbism >= 0 && currentpage != 1)
                        else if (line.Trim() == bism2)
                        {
                            file.WriteLine(@"\centerpars{" + bism2 + "}");
                        }
                        else
                        {
                            if ((i == 1 && currentpage == 1) || (i == 2 && currentpage == 2))
                            {
                                file.WriteLine(@"\startcenter");
                            }


                            if (i == (page.Count - 1))
                            {
                                if (arg == "exact")
                                {
                                    file.WriteLine(line.Replace(" ", "~") + "\\par");
                                }
                                else
                                {
                                    file.WriteLine(line);
                                }


                                if (currentpage == 1 || currentpage == 2)
                                {
                                    file.WriteLine(@"\stopcenter");
                                }
                            }
                            else
                            {


                                if ((nopage + 1) == 567 && (i + 1) == 12)
                                {
                                    line = line.Replace("٢٩", "").Trim();

                                }
                                else if ((nopage + 1) == 567 && (i + 1) == 13)
                                {
                                    line = "٢٩" + " " + line;
                                }

                                if (currentpage == 1 || currentpage == 2)
                                {
                                    file.WriteLine(line + "\\par");
                                }
                                else
                                {
                                    if (arg == "exact")
                                    {
                                        //file.WriteLine(line.Replace(" ", "\\char983040{}") + "\\par");
                                        file.WriteLine(line.Replace(" ", "~") + "\\par");
                                    }
                                    else
                                    {
                                        file.WriteLine(line);
                                    }
                                }


                            }

                        }

                    }


                    if (nopage == 0 || nopage == 1)
                    {
                        file.WriteLine(@"\leavevmode\vfill\page[yes]");
                    }
                    else if (arg == "page" || arg == "exact")
                    {
                        file.WriteLine(@"\par");
                        file.WriteLine(@"\page[yes]");
                    }


                    if (currentpage > 1000)
                        break;
                    currentpage++;
                }

                file.WriteLine("\\stoptext");

                file.Close();

            }


        }
        static string replaceCharcatersFromUthmanicHafsDoc(string line)
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

            // miss letter in لِيَسُـۥٓـُٔوا۟ 
            newline = newline.Replace("\u0633\u064F\u0640\u0654\u064F", "\u0633\u064F\u034F\u08F3\u0653\u0640\u0654\u064F");

            // 0648  ARABIC LETTER WAW + ‎0655  ARABIC HAMZA BELOW -> 0624  ARABIC LETTER WAW WITH HAMZA ABOVE
            newline = newline.Replace("\u0648\u0655", "\u0624");

            //ARABIC LETTER YEH WITH HAMZA ABOVE
            newline = newline.Replace("\u064A\u0655", "\u0626");

            //ARABIC LETTER YEH WITH HAMZA ABOVE foloweed by /0640  ZERO WIDTH JOINER only in "سُورَةُ المَائ‍ِدَةِ"
            newline = newline.Replace("\u0626\u0640", "\u0626");

            // ARABIC SMALL HIGH SEEN + FATHA -> ARABIC SMALL LOW SEEN + FATHA
            newline = newline.Replace("\u06DC\u064E", "\u06E3\u064E");

            //Dont reorder  marks        
            newline = newline.Replace("\u0652\u06DC", "\u0652\u034F\u06DC");

            // change space position after hizb symbol
            string pattern = "\\s*" + "۞" + "\\s*";
            newline = Regex.Replace(newline, pattern, "۞" + " ");

            // Small Madda for Sajda Rule above used to add line above. We dont need this.
            newline = newline.Replace("\u06E4", "");

            /* 0623 : ARABIC LETTER ALEF WITH HAMZA ABOVE + 0653 : ARABIC MADDAH ABOVE 
             * -> 0640 : ARABIC TATWEEL 
             * -> 0654 : ARABIC HAMZA ABOVE
             * -> 064E : ARABIC FATHA
             * -> 0627 : ARABIC LETTER ALEF */
            // disable newline = newline.Replace("\u0623\u0653", "\u0640\u0654\u064E\u0627");                                  
            //newline = newline.Replace("\u064E\u06E2", "\u064B\u06E2"); // 064E : ARABIC FATHA + 06E2 : ARABIC SMALL HIGH MEEM ISOLATED FORM -> 064B :  ARABIC FATHATAN + 06E2
            //newline = newline.Replace("\u064F\u06E2", "\u064C\u06E2");
            //newline = newline.Replace("\u0650\u06E2", "\u064D\u06ED");

            //not needed any more already changed in version 13
            //newline = newline.Replace('\u200D', '\u0640');

            //corrected in version 13 "سُورَةُ التِّينِ" "بِّسۡمِ ٱللَّهِ ٱلرَّحۡمَٰنِ ٱلرَّحِيمِ" 
            //newline = newline.Replace("\u0650\u0651", "\u0651\u0650"); // 0650  ARABIC KASRA + 0651  ARABIC SHADDA -> 0651  ARABIC SHADDA + 0650  ARABIC KASRA                                       

            //newline = newline.Replace("\u065C", "\u06EA"); // 065C  ARABIC VOWEL SIGN DOT BELOW -> 06EA ARABIC EMPTY CENTRE LOW STOP

            //newline = newline.Replace("\u06E1\u06DC", "\u06E1\u034F\u06DC");

            return newline;

        }
        static string setSajdahBar(string line)
        {
            if (line.Contains("خَرُّوا۟\u06E4"))
            {
                line = line.Replace("خَرُّوا۟\u06E4", "\u06E4خَرُّوا۟");
            }

            if (line.Contains("وَلِلَّهِۤ"))
            {
                line = line.Replace("وَلِلَّهِۤ", "\u06E4وَلِلَّهِ");
            }

            Regex r = new Regex(@"(\u06E4(.+?)\u06E4)");
            MatchCollection matches = r.Matches(line);
            foreach (Match m in matches)
            {
                if (line.Contains("لِلْأَذْقَانِۤ"))
                {
                    line = line.Replace("يَخِرُّونَۤ لِلْأَذْقَانِۤ سُجَّدٗاۤ", "\\sajdahbar{" + "يَخِرُّونَۤ لِلْأَذْقَانِۤ سُجَّدٗاۤ" + "}");
                    line = line.Replace("\u06E4", "");
                }
                else if (m.Groups[2].Value == " لَهُۥ")
                {

                    line = line.Replace("يَسْجُدُۤ لَهُۥۤ ", "\\sajdahbar{" + "يَسْجُدُۤ لَهُۥۤ " + "}");
                    line = line.Replace("\u06E4", "");

                }
                else if (m.Groups[2].Value == " رَاكِعٗا")
                {

                    line = line.Replace("وَخَرَّۤ رَاكِعٗاۤ ", "\\sajdahbar{" + "وَخَرَّۤ رَاكِعٗاۤ " + "}");
                    line = line.Replace("\u06E4", "");

                }
                else if (m.Groups[2].Value == " يَسْجُدُوا۟")
                {

                    line = line.Replace("أَلَّاۤ يَسْجُدُوا۟ۤ لِلَّهِ", "\\sajdahbar{" + "أَلَّاۤ يَسْجُدُوا۟ۤ لِلَّهِ" + "}");
                    line = line.Replace("\u06E4", "");

                }
                else if (m.Groups[2].Value == " لِلَّهِ")
                {

                    line = line.Replace("وَٱسْجُدُوا۟ۤ لِلَّهِۤ", "\\sajdahbar{" + "وَٱسْجُدُوا۟ۤ لِلَّهِۤ" + "}");
                    line = line.Replace("فَٱسْجُدُوا۟ۤ لِلَّهِۤ", "\\sajdahbar{" + "فَٱسْجُدُوا۟ۤ لِلَّهِۤ" + "}");
                    line = line.Replace("\u06E4", "");

                }
                else
                {
                    line = line.Replace(m.Groups[0].Value, "\\sajdahbar{" + m.Groups[2].Value + "}");
                }
            }

            r = new Regex(@"((\w+?)\u06E4)");
            matches = r.Matches(line);
            foreach (Match m in matches)
            {
                line = line.Replace(m.Groups[0].Value, "\\sajdahbar{" + m.Groups[2].Value + "}");
            }

            return line;
        }
        static void CompareTanzilAndUthmanicHafsDoc()
        {
            List<List<string>> TanzilPages = new List<List<string>>();
            List<List<string>> UthmanicHafsDocPages = new List<List<string>>();
            List<string> lines = null;
            using (XmlReader reader = XmlReader.Create("input\\allquran_output.xml"))
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
                        TanzilPages.Add(lines);
                    }
                }
            }
            using (XmlReader reader = XmlReader.Create("input\\UthmanicHafs1Ver09.xml"))
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
                        UthmanicHafsDocPages.Add(lines);
                    }
                }
            }
            List<string> TanzilPagesWL = new List<string>();
            List<string> UthmanicHafsDocPagesWL = new List<string>();

            foreach (var page in TanzilPages)
            {
                string text = "";
                foreach (var line in page)
                {

                    if (!line.StartsWith("سُورَةُ "))
                    {
                        if (text != "")
                        {
                            text += " ";
                        }
                        string newline = line;
                        newline = newline.Replace("\u064E\u0640\u0670", "\u064E\u0670");
                        newline = newline.Replace(" \u06D6", "\u06D6");
                        newline = newline.Replace(" \u06D7", "\u06D7");
                        newline = newline.Replace(" \u06D8", "\u06D8");
                        newline = newline.Replace(" \u06D9", "\u06D9");
                        newline = newline.Replace(" \u06DA", "\u06DA");
                        newline = newline.Replace(" \u06DB", "\u06DB");
                        newline = newline.Replace(" \u06DC", "\u06DC");
                        newline = newline.Replace("\u064B\u06ED", "\u0657"); // FATHATAN + SMALL LOW MEEM -> 08F0 ARABIC OPEN FATHATAN
                        newline = newline.Replace("\u064C\u06ED", "\u065E"); //DAMMATAN + SMALL LOW MEEM -> 08F1 ARABIC OPEN DAMMATAN
                        newline = newline.Replace("\u064D\u06E2", "\u0656"); //KASRATAN + SMALL HIGH MEEM -> 08F2 ARABIC OPEN KASRATAN
                        newline = newline.Replace("\u06F0", "\u0660");
                        newline = newline.Replace("\u0650\u0649", "\u0650\u064A");
                        newline = newline.Replace("\u06DE ", "\u06DE");
                        newline = newline.Replace("\u06EB", "\u06EC");
                        newline = newline.Replace("\u06E3", "\u06DC"); // ARABIC SMALL LOW SEEN -> ARABIC SMALL HIGH SEEN


                        //With this become equivalent

                        newline = newline.Replace("\u0649", "\u064A");
                        newline = newline.Replace("\u06D6", "");
                        newline = newline.Replace("\u06D7", "");
                        newline = newline.Replace("\u06D8", "");
                        newline = newline.Replace("\u06D9", "");
                        newline = newline.Replace("\u06DA", "");
                        newline = newline.Replace("\u06DB", "");

                        text += newline;
                    }
                }
                TanzilPagesWL.Add(text);
            }
            List<string> yehmaksoura = new List<string>();
            foreach (var page in UthmanicHafsDocPages)
            {
                string text = "";
                foreach (var line in page)
                {
                    if (line.Contains("\u0649"))
                    {
                        yehmaksoura.Add(line);
                    }
                    if (!line.StartsWith("سُورَةُ "))
                    {
                        if (text != "")
                        {
                            text += " ";
                        }
                        string newline = line.Replace("\u0652", "\u06DF");
                        newline = newline.Replace("\u06E1", "\u0652");
                        newline = newline.Replace("\u0623\u0653", "\u0640\u0654\u064E\u0627");
                        //newline = newline.Replace("\u0641\u0650\u064A ", "\u0641\u0650\u0649 ");                        
                        newline = newline.Replace("\u064E\u06E2", "\u064B\u06E2"); // ARABIC FATHA + ARABIC SMALL HIGH MEEM -> ARABIC FATHATAN + ARABIC SMALL HIGH MEEM 
                        newline = newline.Replace("\u064F\u06E2", "\u064C\u06E2");
                        newline = newline.Replace("\u0650\u06E2", "\u064D\u06ED");
                        newline = newline.Replace("\u200D\u0654", "\u0640\u0654");
                        newline = newline.Replace("\u200D\u06E7", "\u0640\u06E7"); //200D  ZERO WIDTH JOINER - ‎06E7  ARABIC SMALL HIGH YEH -> 0640  ARABIC TATWEEL - 06E7  ARABIC SMALL HIGH YEH
                        newline = newline.Replace("\u200D\u06E8", "\u0640\u06E8"); //200D  ZERO WIDTH JOINER - ‎06E8  ARABIC SMALL HIGH NOON -> 0640  ARABIC TATWEEL - ‎06E8  ARABIC SMALL HIGH NOON 
                        newline = newline.Replace("\u06E4", ""); // Small Madda for Sajda Rule above
                        newline = newline.Replace("\u064E\u0670\u0654\u062A", "\u064E\u0670\u0654\u0652\u062A"); // miss sukun in فَٱدَّٰرَٰٔتُمۡ page 11 line 5
                        newline = newline.Replace("\u0650\u0640\u062C\u0650\u0628", "\u0650\u062C\u0650\u0628"); // suppress tatweel in لِّـجِبۡرِيلَ page 15 line 7 
                        newline = newline.Replace("\u0633\u064F\u0640\u0654\u064F", "\u0633\u064F\u0640\u06E5\u0653\u0640\u0654\u064F"); // miss letter in لِيَسُـۥٓـُٔوا۟ 
                        newline = newline.Replace("\u0648\u0655", "\u0624"); // 0648  ARABIC LETTER WAW + ‎0655  ARABIC HAMZA BELOW -> 0624  ARABIC LETTER WAW WITH HAMZA ABOVE
                        newline = newline.Replace("\u0650\u0651", "\u0651\u0650"); // 0650  ARABIC KASRA + 0651  ARABIC SHADDA -> 0651  ARABIC SHADDA + 0650  ARABIC KASRA                                       
                        newline = newline.Replace("\u065C", "\u06EA"); // 065C  ARABIC VOWEL SIGN DOT BELOW -> 06EA ARABIC EMPTY CENTRE LOW STOP
                        newline = newline.Replace("\u064A\u0655", "\u0626"); //ARABIC LETTER YEH WITH HAMZA ABOVE


                        //With this become equivalent


                        newline = newline.Replace("\u0649", "\u064A");
                        newline = newline.Replace("\u06D6", "");
                        newline = newline.Replace("\u06D7", "");
                        newline = newline.Replace("\u06D8", "");
                        newline = newline.Replace("\u06D9", "");
                        newline = newline.Replace("\u06DA", "");
                        newline = newline.Replace("\u06DB", "");

                        text += newline;
                    }
                }
                UthmanicHafsDocPagesWL.Add(text);
            }
            int equal = 0;
            int different = 0;

            using (System.IO.StreamWriter tanzil = new System.IO.StreamWriter(@"output\tanzil.txt"))
            {
                using (System.IO.StreamWriter uthmani = new System.IO.StreamWriter(@"output\uthmani.txt"))
                {
                    for (int i = 0; i < 604; i++)
                    {
                        //tanzil.WriteLine(TanzilPagesWL[i]);
                        //uthmani.WriteLine(UthmanicHafsDocPagesWL[i]);
                        string pattern = @"(\d+)";
                        string[] result = Regex.Split(TanzilPagesWL[i], pattern);
                        for (int ctr = 0; ctr < result.Length; ctr++)
                        {
                            if (!string.IsNullOrWhiteSpace(result[ctr]))
                                tanzil.WriteLine(result[ctr]);

                        }

                        result = Regex.Split(UthmanicHafsDocPagesWL[i], pattern);
                        for (int ctr = 0; ctr < result.Length; ctr++)
                        {
                            if (!string.IsNullOrWhiteSpace(result[ctr]))
                                uthmani.WriteLine(result[ctr]);

                        }
                        if (TanzilPagesWL[i] != UthmanicHafsDocPagesWL[i])
                        {
                            System.Console.WriteLine("Page " + i + " different");
                            different++;
                        }
                        else
                        {
                            System.Console.WriteLine("Page " + i + " equal");
                            equal++;
                        }
                    }
                }
            }

            System.Console.WriteLine("Equals =  " + equal);
            System.Console.WriteLine("Differents =  " + different);
        }
        static void GenerateXMLFromTanzil()
        {

            String quranText = File.ReadAllText(@"input\allquran.txt");


            dynamic quran = JObject.Parse(quranText);
            var listAyas = quran.quran;

            var xDoc = XDocument.Load(new StreamReader(@"input\quran-data.xml"));

            dynamic root = new ExpandoObject();

            XmlToDynamic.Parse(root, xDoc.Elements().First());

            Console.WriteLine(root.quran.pages.page.Count);

            try
            {
                XmlWriterSettings settings = new XmlWriterSettings
                {
                    Indent = true,
                    IndentChars = "  ",
                    NewLineChars = "\n",
                    NewLineHandling = NewLineHandling.Replace
                };

                using (XmlWriter writer = XmlWriter.Create("input\\allquran_output.xml", settings))
                {
                    writer.WriteStartDocument();
                    writer.WriteStartElement("pages");

                    int numpage = 0;
                    int numline = 0;
                    int currentpage = 1;
                    string suraWord = "سُورَةُ";
                    string bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَـٰنِ ٱلرَّحِيمِ";

                    numpage++;
                    writer.WriteStartElement("Page");
                    writer.WriteAttributeString("num", numpage.ToString());

                    foreach (var sura in root.quran.suras.sura)
                    {

                        int nbayas = Convert.ToInt32(sura.ayas);
                        int startaya = Convert.ToInt32(sura.start);

                        string suraname = suraWord + " " + sura.name;




                        for (int iaya = 1; iaya <= nbayas; iaya++)
                        {

                            string ayacontent = listAyas[startaya];


                            string ayaIndex = ConvertNumerals(iaya.ToString());

                            if (currentpage < 604)
                            {
                                var page = root.quran.pages.page[currentpage];

                                if (page.sura == sura.index && page.aya == iaya.ToString())
                                {
                                    currentpage++;

                                    writer.WriteEndElement();

                                    numpage++;
                                    writer.WriteStartElement("Page");
                                    writer.WriteAttributeString("num", numpage.ToString());
                                }

                            }

                            if (startaya == Convert.ToInt32(sura.start))
                            {
                                numline++;
                                writer.WriteStartElement("Line");
                                writer.WriteAttributeString("num", numline.ToString());
                                writer.WriteValue(suraname);
                                writer.WriteEndElement();
                            }

                            if (startaya != 0 && iaya == 1)
                            {

                                int indexbism = ayacontent.IndexOf(bism + " ");
                                if (indexbism >= 0)
                                {
                                    ayacontent = ayacontent.Remove(indexbism, bism.Length);

                                    numline++;
                                    writer.WriteStartElement("Line");
                                    writer.WriteAttributeString("num", numline.ToString());
                                    writer.WriteValue(bism);
                                    writer.WriteEndElement();
                                }
                            }

                            numline++;
                            writer.WriteStartElement("Line");
                            writer.WriteAttributeString("num", numline.ToString());
                            writer.WriteValue(ayacontent.Trim() + " " + ayaIndex);
                            writer.WriteEndElement();

                            startaya++;

                        }


                    }

                    writer.WriteEndElement();
                    writer.WriteEndDocument();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString());
            }
        }
        static void GenerateXMLfromUthmanicHafsDoc(string fileName, string outputFile)
        {
            generateXML(readPages(fileName), outputFile);
        }

        static List<List<string>> readPages(string fileName)
        {
            Document docs = null;
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
                    System.Runtime.InteropServices.Marshal.ReleaseComObject(docs);
                }

                if (word != null)
                {
                    word.Quit(false);
                    System.Runtime.InteropServices.Marshal.ReleaseComObject(word);
                }
                docs = null;
                word = null;
                GC.Collect();
            }

            return Pages;
        }

        static void generateXML(List<List<string>> Pages, string outputFile)
        {

            XmlWriterSettings settings = new XmlWriterSettings
            {
                Indent = true,
                IndentChars = "  ",
                NewLineChars = "\n",
                NewLineHandling = NewLineHandling.Replace
            };

            using (XmlWriter writer = XmlWriter.Create(outputFile, settings))
            {
                writer.WriteStartDocument();
                writer.WriteStartElement("pages");

                int numpage = 0;

                foreach (var lines in Pages)
                {
                    numpage++;
                    writer.WriteStartElement("Page");
                    writer.WriteAttributeString("num", numpage.ToString());
                    int numline = 0;
                    foreach (var line in lines)
                    {
                        numline++;
                        writer.WriteStartElement("Line");
                        writer.WriteAttributeString("num", numline.ToString());
                        writer.WriteValue(line);
                        writer.WriteEndElement();

                    }

                    writer.WriteEndElement();

                }

                writer.WriteEndElement();
                writer.WriteEndDocument();
            }

        }
        static void GenerateTexFileFromTanzil()
        {

            String quranText = File.ReadAllText(@"input\allquran.txt");


            dynamic quran = JObject.Parse(quranText);
            var listAyas = quran.quran;

            var xDoc = XDocument.Load(new StreamReader(@"input\quran-data.xml"));

            dynamic root = new ExpandoObject();

            XmlToDynamic.Parse(root, xDoc.Elements().First());

            Console.WriteLine(root.quran.pages.page.Count);



            string outputfile = @"D:\projects\Fonts\texexamples\alqalam\metapost\quran.tex";
            string suraWord = "سُورَةُ";
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(outputfile))
            {


                file.WriteLine(@"\environment ara-sty");

                file.WriteLine(@"\directlua{ require('mobdebug').start()}");
                file.WriteLine(@"\usemodule[m-alqalam]");
                file.WriteLine(@"\usemodule[myfont-otc]");
                file.WriteLine(@"\input frame");
                file.WriteLine();

                file.WriteLine(@"\starttext");

                file.WriteLine();
                file.WriteLine(@"\alqalam");
                file.WriteLine(@"\MedinaMushafFont");
                //file.WriteLine(@"\setupinterlinespace[line=50pt]");
                file.WriteLine(@"\setupinterlinespace[line=1.55ex]");
                file.WriteLine();

                int breakafter = 0;
                int currentpage = 1;

                foreach (var sura in root.quran.suras.sura)
                {

                    int nbayas = Convert.ToInt32(sura.ayas);
                    int startaya = Convert.ToInt32(sura.start);
                    string bism = "بِسْمِ ٱللَّهِ ٱلرَّحْمَـٰنِ ٱلرَّحِيمِ";

                    file.WriteLine(@"\chapter{" + suraWord + " " + sura.name + "}");
                    file.WriteLine(@"\startlinealignment[middle]");
                    file.WriteLine(@"\ArabicGlobalDir ");
                    file.WriteLine(suraWord + " " + sura.name);
                    file.WriteLine(@"\stoplinealignment\par");


                    string ayacontent = "";

                    if (startaya != 0)
                    {
                        ayacontent = listAyas[startaya];

                        int indexbism = ayacontent.IndexOf(bism + " ");
                        if (indexbism >= 0)
                        {
                            ayacontent = ayacontent.Remove(indexbism, bism.Length);

                            file.WriteLine(@"\startlinealignment[middle]");
                            file.WriteLine(@"\ArabicGlobalDir ");
                            file.WriteLine(bism);
                            file.WriteLine(@"\stoplinealignment\par");

                        }
                    }




                    file.WriteLine(@"{");
                    file.WriteLine(@"\ArabicGlobalDir");


                    for (int iaya = 1; iaya <= nbayas; iaya++)
                    {
                        if (iaya != 1 || ayacontent == "")
                            ayacontent = listAyas[startaya];


                        string ayaIndex = ConvertNumerals(iaya.ToString());

                        if (currentpage < 604)
                        {
                            var page = root.quran.pages.page[currentpage];

                            if (page.sura == sura.index && page.aya == iaya.ToString())
                            {
                                currentpage++;

                                /*
                                file.WriteLine(@"\par}");
                                file.WriteLine(@"\page[yes]");
                                file.WriteLine(@"{");
                                file.WriteLine(@"\ArabicGlobalDir");*/
                            }

                        }





                        file.Write(ayacontent + " " + ayaIndex + " ");
                        startaya++;
                    }
                    file.WriteLine(@"\par}");

                    breakafter++;
                    if (breakafter == 2)
                        //if (breakafter == 1000)
                        break;
                }

                file.WriteLine("\\stoptext");

                file.Close();

            }



            /*
            string suraWord = "سُورَةُ";
            string outputfile = @"quran.tex";
            using (System.IO.StreamWriter file = new System.IO.StreamWriter(outputfile))
            {
                file.WriteLine(@"\environment ara-sty");

                file.WriteLine(@"\directlua{ require('mobdebug').start()}");

                file.WriteLine(@"\usemodule[m - alqalam]");

                file.WriteLine(@"\definefont[alqalam][demo@alqalam at 30pt]");
                file.WriteLine(@"\setupbodyfont[alqalam, 30pt]");
                file.WriteLine(@"\ArabicGlobalDir");
                file.WriteLine(@"\setupinterlinespace[line = 2.5ex]");

                file.WriteLine(@"\starttext");

                XmlDocument doc = new XmlDocument();
                doc.Load("quran-uthmani.xml");
                int i = 0;

                foreach (XmlNode node in doc.DocumentElement.ChildNodes)
                {

                    string suraName = node.Attributes["name"].InnerText;
                    file.WriteLine("\n\\startlinealignment [middle]");
                    file.WriteLine("\n\\ArabicGlobalDir");
                    file.WriteLine(suraWord + " " + suraName);
                    file.WriteLine("\\stoplinealignment\n");
                    foreach (XmlNode aya in node.ChildNodes)
                    {
                        string ayaText = aya.Attributes["text"].InnerText;
                        string ayaIndex = ConvertNumerals(aya.Attributes["index"].InnerText);

                        file.WriteLine(ayaText + " " + ayaIndex);

                    }
                    i++;
                    if (i == 2)
                        break;

                }

                file.WriteLine("\\stoptext");

                file.Close();

            }*/


        }
        public static string ConvertNumerals(string input)
        {

            return input.Replace('0', '\u06f0')
                    .Replace('1', '\u0661')
                    .Replace('2', '\u0662')
                    .Replace('3', '\u0663')
                    .Replace('4', '\u0664')
                    .Replace('5', '\u0665')
                    .Replace('6', '\u0666')
                    .Replace('7', '\u0667')
                    .Replace('8', '\u0668')
                    .Replace('9', '\u0669');

        }
    }

}
