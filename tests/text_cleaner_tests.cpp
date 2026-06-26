#include "../src/text_cleaner.h"

#include <iostream>

int wmain() {
    CleanOptions options;
    options.stripMarkdownLinks = true;

    const std::wstring source =
        L"This is a para-\n"
        L"graph copied from a\n"
        L"PDF file.   \n"
        L"\u4e2d \u6587 \u4e4b \u95f4 \u6709 \u7a7a \u683c\n"
        L"hello     world\n"
        L"[OpenAI](https://openai.com)";

    const std::wstring expected =
        L"This is a paragraph copied from a PDF file.\n"
        L"\u4e2d\u6587\u4e4b\u95f4\u6709\u7a7a\u683c\n"
        L"hello world\n"
        L"OpenAI";

    const std::wstring actual = CleanText(source, options);
    if (actual != expected) {
        std::wcerr << L"Unexpected cleaned text:\n" << actual << L"\n";
        return 1;
    }

    return 0;
}
