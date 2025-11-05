#ifndef LANGUAGE_DETECTOR_H
#define LANGUAGE_DETECTOR_H

#include <string>

enum class Language {
    C,
    CPP,
    JAVA,
    PYTHON,
    UNKNOWN
};

class LanguageDetector {
public:
    static Language detectLanguage(const std::string& filename);
    static Language detectByContent(const std::string& content);
    static std::string languageToString(Language lang);

private:
    static Language detectByExtension(const std::string& filename);
    static Language detectByShebang(const std::string& content);
    static Language detectBySyntax(const std::string& content);
};

#endif // LANGUAGE_DETECTOR_H
