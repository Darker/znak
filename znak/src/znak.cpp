#include "znak.h"
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace znak;

// using Letters = CharacterSet<"abcdefghijklmnopqrstuvwxyz">;
// using UpperCaseLetters = CharacterSet<"ABCDEFGHIJKLMNOPQRSTUVWXYZ">;
// using Numbers = CharacterSet<"123456789">;

bool checkComplete(Parser& p, std::ostream& out, const std::string& source)
{
    if(p.valid())
    {
        out<<"Parsing complete. Results follow...\n";
        range results = p.parsedRange();
        results.print(out, source);
        out<<"END\n";
        return true;
    }
    else
    {
        out<<"Unexpected end of data!\n";
        return false;
    }

}

bool runParser(sptr<Parser> parser, const std::string& data)
{
    parser->reset();

    std::stringbuf pbuf(data);
    std::stringstream all;
    bool done = false;
    size_t off = 0;
    while (!done) {
        //std::cout << "Enter an character (or esc to quit): " << std::endl;
        char c;


        if (pbuf.sgetc() == EOF) 
        {
            return checkComplete(parser, std::cout, all.str());
        }
        c = pbuf.sbumpc();
        if (c == 0x1b) {
            return checkComplete(parser, std::cout, all.str());
        } else {
            all << c;
            if(!parser->check(off, c))
            {
                if(parser->valid())
                {
                    return checkComplete(parser, std::cout, all.str());
                }
                else 
                {
                    std::cout<<"Parser rejected a character, invalid input probably or something.\n";
                    std::cout<<"Offset: "<<off<<" character: 0x" << std::setw(2) << std::setfill('0') << std::hex << int(c)<< " '"<<c<<"'" << std::endl;
                    return false;
                }
            }
            std::cout << "You entered character 0x" << std::setw(2) << std::setfill('0') << std::hex << int(c)<< " '"<<c<<"'" << std::endl;
        }
        ++off;
    }
    return false;
}

sptr<Parser> createStringLiteral()
{
  const auto quote = CharacterSimple<'"'>();
  return Sequence("string").add
  (
    MatchRange::once("open_quote", quote),
    Repeat("contents", 1, Repeat::MATCH_MAX,
      First("literal_or_escaped")
      .add
      (
        MatchRange::unlimited("characters", ExceptCharacters("\\\"\n")),
        Sequence("escaped_character")
        .add
        (
          MatchRange::once("escape_symbol", CharacterSimple<'\\'>()),
          MatchRange::once("escaped_char", Anything())
        )
      )
    ),
    MatchRange::once("close_quote", quote)
  );
}

// sptr<Parser> createFloatLiteral()
// {
//   const auto dot = CharacterSimple<'.'>();
//   return Sequence("string").add
//   (
//     MatchRange::unlimited("decimals", chars::NUMBERS),
//     Repeat("contents", 1, Repeat::MATCH_MAX,
//       First("literal_or_escaped")
//       .add
//       (
//         MatchRange::unlimited("characters", ExceptCharacters("\\\"\n")),
//         Sequence("escaped_character")
//         .add
//         (
//           MatchRange::once("escape_symbol", CharacterSimple<'\\'>()),
//           MatchRange::once("escaped_char", Anything())
//         )
//       )
//     ),
//     MatchRange::once("close_quote", quote)
//   );
// }

int parseStringTest()
{
    sptr<Parser> stringLiteral = createStringLiteral();

    std::string testData = "\"blabloa dsa \\\\ dsads\\\" dsa\\\\\\\"  \"";


    return (int)runParser(sptr<Parser>::borrow(stringLiteral), testData);
}


int parseFloatTest()
{
    Sequence floating_point("floating_point");

    floating_point
        .add(
            MatchRange("decimals", 1, MatchRange::MATCH_MAX, chars::NUMBERS),
            MatchRange("dot",1,1)
                .add(CharacterSimple<'.'>()),
            MatchRange("fraction", 1, MatchRange::MATCH_MAX, chars::NUMBERS)
        );

    return (int)runParser(sptr<Parser>::borrow(floating_point), "1.25");
}


int main(int argc, const char** argv)
{
    //static_assert(std::is_base_of<Matcher, CharacterSet>::value, "Character set is subclass of matcher");
    //static_assert(std::is_convertible<CharacterSet, Matcher>::value, "Character set is subclass of matcher");
    // MatchRange dot = MatchRange(1,9999,"dot").add(CharacterSimple<'.'>());
    // Sequence floating_point("floating_point");
    // floating_point.add
    // (
    //     MatchRange(1,9999,"decimals", chars::NUMBERS),
    //     new MatchRange(1,9999,"decimals", chars::NUMBERS),
    //     sptr<Parser>::borrow(dot)
    // );
    // floating_point
    //     .add(
    //         MatchRange(1,9999,"decimals", chars::NUMBERS),
    //         MatchRange(1,9999,"dot")
    //             .add(CharacterSimple<'.'>()),
    //         MatchRange(1,9999,"fraction", chars::NUMBERS)
    //     );
    ;
        // .add((new MatchRange(1,9999,"dot"))->add(new CharacterSimple<'.'>()))
        // .add(new MatchRange(1,9999,"fraction", chars::NUMBERS));

    // Sequence floating_point("floating_point");
    // floating_point
    //     .add(new MatchRange(1,9999,"decimals", chars::NUMBERS))
    //     .add((new MatchRange(1,9999,"dot"))->add(new CharacterSimple<'.'>()))
    //     .add(new MatchRange(1,9999,"fraction", chars::NUMBERS));


    return parseStringTest();
}