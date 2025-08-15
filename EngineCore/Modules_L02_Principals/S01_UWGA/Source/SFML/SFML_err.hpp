// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_UWGA_SFML_ERR_HPP
#define UHOBGOBLIN_UWGA_SFML_ERR_HPP

#include <SFML/System.hpp>

#include <sstream>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace uwga {

class SFMLErrorCatcher {
public:
    SFMLErrorCatcher() {
        _previous = sf::err().rdbuf(_oss.rdbuf());
    }

    ~SFMLErrorCatcher() {
        sf::err().rdbuf(_previous);
    }

    std::string getErrorMessage() const {
        std::string result = _oss.str();
        if ((result.size() > 0) && (result[result.size() - 1] == '\n')) {
            result.pop_back();
        }
        return result;
    }

private:
    std::streambuf*    _previous;
    std::ostringstream _oss;
};

} // namespace uwga
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_UWGA_SFML_ERR_HPP
