#include "stdafx.h"
#include "WeechatString.h"

#include <sstream>
#include <algorithm>

const std::string CWeechatString::RE_COLOR_ATTRS("[*!/_|]*");
const std::string CWeechatString::RE_COLOR_STD("(?:" + RE_COLOR_ATTRS + "\\d{2})");
const std::string CWeechatString::RE_COLOR_EXT("(?:@" + RE_COLOR_ATTRS + "\\d{5})");
const std::string CWeechatString::RE_COLOR_ANY("(?:" + RE_COLOR_STD + "|" + RE_COLOR_EXT + ")");
// \x19: color code, \x1A: set attribute, \x1B: remove attribute, \x1C: reset
const std::regex CWeechatString::RE_COLOR(
    "("
        "\x19(?:"
            "\\d{2}"
            "|F" + RE_COLOR_ANY + ""
            "|B\\d{2}"
            "|B@\\d{5}"
            "|E"
            "|\\\\*" + RE_COLOR_ANY + "(," + RE_COLOR_ANY + ")?"
            "|@\\d{5}"
            "|b."
            "|\x1C"
        ")"
    ")"
    "|\x1A."
    "|\x1B."
    "|\x1C"
);

CWeechatString::~CWeechatString()
{
}

std::sregex_token_iterator CWeechatString::begin() const
{
    return std::sregex_token_iterator(m_wctext.begin(), m_wctext.end(), RE_COLOR, { -1, 0 });
}

std::sregex_token_iterator CWeechatString::end() const
{
    return std::sregex_token_iterator();
}

std::string escapeRtf(std::string string)
{
    std::ostringstream out;

    for (const unsigned char& ch : string)
    {
        switch (ch)
        {
        case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
        case 10: case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19:
        case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: case 29:
        case 30: case 31: break;
        case 32: out << "\\'20"; break;
        case 33: out << "\\'21"; break;
        case 34: out << "\\'22"; break;
        case 35: out << "\\'23"; break;
        case 36: out << "\\'24"; break;
        case 37: out << "\\'25"; break;
        case 38: out << "\\'26"; break;
        case 39: out << "\\'27"; break;
        case 40: out << "\\'28"; break;
        case 41: out << "\\'29"; break;
        case 42: out << "\\'2a"; break;
        case 43: out << "\\'2b"; break;
        case 44: out << "\\'2c"; break;
        case 45: out << "\\'2d"; break;
        case 46: out << "\\'2e"; break;
        case 47: out << "\\'2f"; break;
        case 48: out << "\\'30"; break;
        case 49: out << "\\'31"; break;
        case 50: out << "\\'32"; break;
        case 51: out << "\\'33"; break;
        case 52: out << "\\'34"; break;
        case 53: out << "\\'35"; break;
        case 54: out << "\\'36"; break;
        case 55: out << "\\'37"; break;
        case 56: out << "\\'38"; break;
        case 57: out << "\\'39"; break;
        case 58: out << "\\'3a"; break;
        case 59: out << "\\'3b"; break;
        case 60: out << "\\'3c"; break;
        case 61: out << "\\'3d"; break;
        case 62: out << "\\'3e"; break;
        case 63: out << "\\'3f"; break;
        case 64: out << "\\'40"; break;
        case 65: out << "\\'41"; break;
        case 66: out << "\\'42"; break;
        case 67: out << "\\'43"; break;
        case 68: out << "\\'44"; break;
        case 69: out << "\\'45"; break;
        case 70: out << "\\'46"; break;
        case 71: out << "\\'47"; break;
        case 72: out << "\\'48"; break;
        case 73: out << "\\'49"; break;
        case 74: out << "\\'4a"; break;
        case 75: out << "\\'4b"; break;
        case 76: out << "\\'4c"; break;
        case 77: out << "\\'4d"; break;
        case 78: out << "\\'4e"; break;
        case 79: out << "\\'4f"; break;
        case 80: out << "\\'50"; break;
        case 81: out << "\\'51"; break;
        case 82: out << "\\'52"; break;
        case 83: out << "\\'53"; break;
        case 84: out << "\\'54"; break;
        case 85: out << "\\'55"; break;
        case 86: out << "\\'56"; break;
        case 87: out << "\\'57"; break;
        case 88: out << "\\'58"; break;
        case 89: out << "\\'59"; break;
        case 90: out << "\\'5a"; break;
        case 91: out << "\\'5b"; break;
        case 92: out << "\\'5c"; break;
        case 93: out << "\\'5d"; break;
        case 94: out << "\\'5e"; break;
        case 95: out << "\\'5f"; break;
        case 96: out << "\\'60"; break;
        case 97: out << "\\'61"; break;
        case 98: out << "\\'62"; break;
        case 99: out << "\\'63"; break;
        case 100: out << "\\'64"; break;
        case 101: out << "\\'65"; break;
        case 102: out << "\\'66"; break;
        case 103: out << "\\'67"; break;
        case 104: out << "\\'68"; break;
        case 105: out << "\\'69"; break;
        case 106: out << "\\'6a"; break;
        case 107: out << "\\'6b"; break;
        case 108: out << "\\'6c"; break;
        case 109: out << "\\'6d"; break;
        case 110: out << "\\'6e"; break;
        case 111: out << "\\'6f"; break;
        case 112: out << "\\'70"; break;
        case 113: out << "\\'71"; break;
        case 114: out << "\\'72"; break;
        case 115: out << "\\'73"; break;
        case 116: out << "\\'74"; break;
        case 117: out << "\\'75"; break;
        case 118: out << "\\'76"; break;
        case 119: out << "\\'77"; break;
        case 120: out << "\\'78"; break;
        case 121: out << "\\'79"; break;
        case 122: out << "\\'7a"; break;
        case 123: out << "\\'7b"; break;
        case 124: out << "\\'7c"; break;
        case 125: out << "\\'7d"; break;
        case 126: out << "\\'7e"; break;
        case 127: out << "\\'7f"; break;
        case 128: out << "\\'80"; break;
        case 129: out << "\\'81"; break;
        case 130: out << "\\'82"; break;
        case 131: out << "\\'83"; break;
        case 132: out << "\\'84"; break;
        case 133: out << "\\'85"; break;
        case 134: out << "\\'86"; break;
        case 135: out << "\\'87"; break;
        case 136: out << "\\'88"; break;
        case 137: out << "\\'89"; break;
        case 138: out << "\\'8a"; break;
        case 139: out << "\\'8b"; break;
        case 140: out << "\\'8c"; break;
        case 141: out << "\\'8d"; break;
        case 142: out << "\\'8e"; break;
        case 143: out << "\\'8f"; break;
        case 144: out << "\\'90"; break;
        case 145: out << "\\'91"; break;
        case 146: out << "\\'92"; break;
        case 147: out << "\\'93"; break;
        case 148: out << "\\'94"; break;
        case 149: out << "\\'95"; break;
        case 150: out << "\\'96"; break;
        case 151: out << "\\'97"; break;
        case 152: out << "\\'98"; break;
        case 153: out << "\\'99"; break;
        case 154: out << "\\'9a"; break;
        case 155: out << "\\'9b"; break;
        case 156: out << "\\'9c"; break;
        case 157: out << "\\'9d"; break;
        case 158: out << "\\'9e"; break;
        case 159: out << "\\'9f"; break;
        case 160: out << "\\~"; break;
        case 161: out << "\\'a1"; break;
        case 162: out << "\\'a2"; break;
        case 163: out << "\\'a3"; break;
        case 164: out << "\\'a4"; break;
        case 165: out << "\\'a5"; break;
        case 166: out << "\\'a6"; break;
        case 167: out << "\\'a7"; break;
        case 168: out << "\\'a8"; break;
        case 169: out << "\\'a9"; break;
        case 170: out << "\\'aa"; break;
        case 171: out << "\\'ab"; break;
        case 172: out << "\\'ac"; break;
        case 173: out << "\\-"; break;
        case 174: out << "\\'ae"; break;
        case 175: out << "\\'af"; break;
        case 176: out << "\\'b0"; break;
        case 177: out << "\\'b1"; break;
        case 178: out << "\\'b2"; break;
        case 179: out << "\\'b3"; break;
        case 180: out << "\\'b4"; break;
        case 181: out << "\\'b5"; break;
        case 182: out << "\\'b6"; break;
        case 183: out << "\\'b7"; break;
        case 184: out << "\\'b8"; break;
        case 185: out << "\\'b9"; break;
        case 186: out << "\\'ba"; break;
        case 187: out << "\\'bb"; break;
        case 188: out << "\\'bc"; break;
        case 189: out << "\\'bd"; break;
        case 190: out << "\\'be"; break;
        case 191: out << "\\'bf"; break;
        case 192: out << "\\'c0"; break;
        case 193: out << "\\'c1"; break;
        case 194: out << "\\'c2"; break;
        case 195: out << "\\'c3"; break;
        case 196: out << "\\'c4"; break;
        case 197: out << "\\'c5"; break;
        case 198: out << "\\'c6"; break;
        case 199: out << "\\'c7"; break;
        case 200: out << "\\'c8"; break;
        case 201: out << "\\'c9"; break;
        case 202: out << "\\'ca"; break;
        case 203: out << "\\'cb"; break;
        case 204: out << "\\'cc"; break;
        case 205: out << "\\'cd"; break;
        case 206: out << "\\'ce"; break;
        case 207: out << "\\'cf"; break;
        case 208: out << "\\'d0"; break;
        case 209: out << "\\'d1"; break;
        case 210: out << "\\'d2"; break;
        case 211: out << "\\'d3"; break;
        case 212: out << "\\'d4"; break;
        case 213: out << "\\'d5"; break;
        case 214: out << "\\'d6"; break;
        case 215: out << "\\'d7"; break;
        case 216: out << "\\'d8"; break;
        case 217: out << "\\'d9"; break;
        case 218: out << "\\'da"; break;
        case 219: out << "\\'db"; break;
        case 220: out << "\\'dc"; break;
        case 221: out << "\\'dd"; break;
        case 222: out << "\\'de"; break;
        case 223: out << "\\'df"; break;
        case 224: out << "\\'e0"; break;
        case 225: out << "\\'e1"; break;
        case 226: out << "\\'e2"; break;
        case 227: out << "\\'e3"; break;
        case 228: out << "\\'e4"; break;
        case 229: out << "\\'e5"; break;
        case 230: out << "\\'e6"; break;
        case 231: out << "\\'e7"; break;
        case 232: out << "\\'e8"; break;
        case 233: out << "\\'e9"; break;
        case 234: out << "\\'ea"; break;
        case 235: out << "\\'eb"; break;
        case 236: out << "\\'ec"; break;
        case 237: out << "\\'ed"; break;
        case 238: out << "\\'ee"; break;
        case 239: out << "\\'ef"; break;
        case 240: out << "\\'f0"; break;
        case 241: out << "\\'f1"; break;
        case 242: out << "\\'f2"; break;
        case 243: out << "\\'f3"; break;
        case 244: out << "\\'f4"; break;
        case 245: out << "\\'f5"; break;
        case 246: out << "\\'f6"; break;
        case 247: out << "\\'f7"; break;
        case 248: out << "\\'f8"; break;
        case 249: out << "\\'f9"; break;
        case 250: out << "\\'fa"; break;
        case 251: out << "\\'fb"; break;
        case 252: out << "\\'fc"; break;
        case 253: out << "\\'fd"; break;
        case 254: out << "\\'fe"; break;
        case 255: out << "\\'ff"; break;
        }
    }

    return out.str();
}

CStringA CWeechatString::decodeRtf() const
{
    std::string wctext = CT2A(m_raw);
    std::ostringstream rtftext;

    auto begin = std::sregex_token_iterator(m_wctext.begin(), m_wctext.end(), RE_COLOR, { -1, 0 });
    auto end = std::sregex_token_iterator();

    for (std::sregex_token_iterator i = begin; i != end; ++i)
    {
        std::ssub_match match = *i;
        std::string token = match.str();
        switch (token[0])
        {
        case '\x19':
            break;
        case '\x1A':
            break;
        case '\x1B':
            break;
        case '\x1C':
            break;
        default:
            rtftext << escapeRtf(token);
            break;
        }
    }

    return rtftext.str().c_str();
}

CString CWeechatString::decodePlain() const
{
    std::ostringstream plaintext;

    auto begin = std::sregex_token_iterator(m_wctext.begin(), m_wctext.end(), RE_COLOR, { -1, 0 });
    auto end = std::sregex_token_iterator();

    for (std::sregex_token_iterator i = begin; i != end; ++i)
    {
        std::ssub_match match = *i;
        std::string token = match.str();
        switch (token[0])
        {
        case '\x19':
            break;
        case '\x1A':
            break;
        case '\x1B':
            break;
        case '\x1C':
            break;
        default:
            std::replace(token.begin(), token.end(), '\t', ' ');
            plaintext << token;
            break;
        }
    }

    return CString(plaintext.str().c_str());
}
