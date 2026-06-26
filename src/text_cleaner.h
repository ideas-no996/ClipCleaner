#pragma once

#include <string>

struct CleanOptions {
    bool mergeEnglishPdfLineBreaks = true;
    bool removeSpacesBetweenChinese = true;
    bool collapseMultipleSpaces = true;
    bool trimTrailingSpaces = true;
    bool stripMarkdownLinks = false;
};

std::wstring CleanText(const std::wstring& input, const CleanOptions& options);
