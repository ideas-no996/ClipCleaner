#include "text_cleaner.h"

#include <cwctype>

namespace {

bool IsAsciiAlpha(wchar_t ch) {
    return (ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z');
}

bool IsAsciiDigit(wchar_t ch) {
    return ch >= L'0' && ch <= L'9';
}

bool IsChinese(wchar_t ch) {
    return ch >= 0x4E00 && ch <= 0x9FFF;
}

bool IsMergeLeft(wchar_t ch) {
    return IsAsciiAlpha(ch) || IsAsciiDigit(ch) || ch == L',' || ch == L';' ||
           ch == L':' || ch == L')' || ch == L']' || ch == L'"' ||
           ch == L'\'';
}

bool IsMergeRight(wchar_t ch) {
    return IsAsciiAlpha(ch) || ch == L'(';
}

bool IsSpaceOrTab(wchar_t ch) {
    return ch == L' ' || ch == L'\t';
}

std::wstring NormalizeNewlines(const std::wstring& input) {
    std::wstring out;
    out.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == L'\r') {
            if (i + 1 < input.size() && input[i + 1] == L'\n') {
                ++i;
            }
            out.push_back(L'\n');
        } else {
            out.push_back(input[i]);
        }
    }

    return out;
}

std::wstring StripMarkdownLinks(const std::wstring& input) {
    std::wstring out;
    out.reserve(input.size());

    for (size_t i = 0; i < input.size();) {
        if (input[i] != L'[') {
            out.push_back(input[i++]);
            continue;
        }

        const size_t closeBracket = input.find(L']', i + 1);
        if (closeBracket == std::wstring::npos ||
            closeBracket + 1 >= input.size() ||
            input[closeBracket + 1] != L'(') {
            out.push_back(input[i++]);
            continue;
        }

        const size_t closeParen = input.find(L')', closeBracket + 2);
        if (closeParen == std::wstring::npos) {
            out.push_back(input[i++]);
            continue;
        }

        out.append(input, i + 1, closeBracket - i - 1);
        i = closeParen + 1;
    }

    return out;
}

std::wstring MergeEnglishPdfLineBreaks(const std::wstring& input) {
    std::wstring out;
    out.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == L'-' &&
            i > 0 &&
            i + 1 < input.size() &&
            input[i + 1] == L'\n' &&
            IsAsciiAlpha(input[i - 1]) &&
            i + 2 < input.size() &&
            IsAsciiAlpha(input[i + 2])) {
            ++i;
            continue;
        }

        if (input[i] == L'\n' &&
            !out.empty() &&
            i + 1 < input.size() &&
            IsMergeLeft(out.back()) &&
            IsMergeRight(input[i + 1])) {
            out.push_back(L' ');
            continue;
        }

        out.push_back(input[i]);
    }

    return out;
}

std::wstring RemoveSpacesBetweenChinese(const std::wstring& input) {
    std::wstring out;
    out.reserve(input.size());

    for (size_t i = 0; i < input.size(); ++i) {
        if (std::iswspace(input[i]) &&
            !out.empty() &&
            i + 1 < input.size() &&
            IsChinese(out.back()) &&
            IsChinese(input[i + 1])) {
            continue;
        }
        out.push_back(input[i]);
    }

    return out;
}

std::wstring CollapseMultipleSpaces(const std::wstring& input) {
    std::wstring out;
    out.reserve(input.size());

    bool inRun = false;
    for (wchar_t ch : input) {
        if (IsSpaceOrTab(ch)) {
            if (!inRun) {
                out.push_back(L' ');
                inRun = true;
            }
        } else {
            out.push_back(ch);
            inRun = false;
        }
    }

    return out;
}

std::wstring TrimTrailingSpaces(const std::wstring& input) {
    std::wstring out;
    out.reserve(input.size());

    size_t lineStart = 0;
    while (lineStart < input.size()) {
        size_t lineEnd = input.find(L'\n', lineStart);
        const bool hasNewline = lineEnd != std::wstring::npos;
        if (!hasNewline) {
            lineEnd = input.size();
        }

        size_t trimmedEnd = lineEnd;
        while (trimmedEnd > lineStart && IsSpaceOrTab(input[trimmedEnd - 1])) {
            --trimmedEnd;
        }

        out.append(input, lineStart, trimmedEnd - lineStart);
        if (hasNewline) {
            out.push_back(L'\n');
            lineStart = lineEnd + 1;
        } else {
            break;
        }
    }

    return out;
}

}  // namespace

std::wstring CleanText(const std::wstring& input, const CleanOptions& options) {
    std::wstring text = NormalizeNewlines(input);

    if (options.mergeEnglishPdfLineBreaks) {
        text = MergeEnglishPdfLineBreaks(text);
    }
    if (options.stripMarkdownLinks) {
        text = StripMarkdownLinks(text);
    }
    if (options.removeSpacesBetweenChinese) {
        text = RemoveSpacesBetweenChinese(text);
    }
    if (options.collapseMultipleSpaces) {
        text = CollapseMultipleSpaces(text);
    }
    if (options.trimTrailingSpaces) {
        text = TrimTrailingSpaces(text);
    }

    return text;
}
