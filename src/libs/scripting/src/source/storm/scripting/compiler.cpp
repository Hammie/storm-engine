#include "storm/scripting/compiler.hpp"

namespace storm::scripting {

size_t Compiler::getNextTokenLength(const std::string_view &input, bool keep_control_symbols)
{
    size_t size = 0;
    bool bDot = false;
    bool bOnlyDigit = true;
    const char* pointer = input.data();
    do
    {
        auto sym = *pointer;

        if (sym == '.')
            bDot = true;

        if (sym == 'f')
        {
            if (size > 1)
                if (bDot && bOnlyDigit)
                {
                    size++;
                    return size;
                }
        }

        if (sym < 0x30 || sym > 0x39)
        {
            if (sym != '.')
                bOnlyDigit = false;
        }
        if (keep_control_symbols)
        {
            if (sym == 0x9 || sym == 0x20)
            {
                if (size == 0)
                    return 1;
                return size;
            }
        }
        if (sym <= 0x20)
            return size;
        if (sym == ';')
            return size;
        /*if(sym == 'f')
        {
          if(bDot && bOnlyDigit)
          {
            size++;
            return size;
          }
        }*/

        if (sym == '{' || sym == '}' || sym == ':' || sym == '(' || sym == ')' || sym == '[' || sym == ']' ||
            /*sym == '*' || */ sym == '^' || sym == '%' || sym == ',')
        {
            if (size == 0)
                return 1;
            return size;
        }

        if (sym == '*')
        {
            pointer++;
            sym = *pointer;
            switch (sym)
            {
            case '=':
                if (size == 0)
                    return 2;
                return size;
            default:
                if (size == 0)
                    return 1;
                return size;
            }
        }

        if (sym == '.')
        {
            if (size == 0)
                return 1;
            if (!bOnlyDigit)
                return size;
            /*pointer--;
            sym = *pointer;
            if(sym < 0x30 || sym > 0x39) return size;
            pointer++;*/
        }

        if (sym == '>' || sym == '<')
        {
            pointer++;
            sym = *pointer;
            if (sym != '=')
            {
                if (size == 0)
                    return 1;
                return size;
            }
            if (size == 0)
                return 2;
            return size;
        }

        if (sym == '=')
        {
            pointer++;
            sym = *pointer;
            if (sym != '=')
            {
                if (size == 0)
                    return 1;
                return size;
            }
            if (size == 0)
                return 2;
            return size;
        }
        if (sym == '!')
        {
            pointer++;
            sym = *pointer;
            if (sym != '=')
            {
                if (size == 0)
                    return 1;
                return size;
            }
            if (size == 0)
                return 2;
            return size;
        }
        if (sym == '&')
        {
            pointer++;
            sym = *pointer;
            if (sym != '&')
            {
                if (size == 0)
                    return 1;
                return size;
            }
            if (size == 0)
                return 2;
            return size;
        }
        if (sym == '+')
        {
            pointer++;
            sym = *pointer;
            switch (sym)
            {
            case '=':
            case '+':
                if (size == 0)
                    return 2;
                return size;
            default:
                if (size == 0)
                    return 1;
                return size;
            }
        }
        if (sym == '-')
        {
            pointer++;
            sym = *pointer;
            switch (sym)
            {
            case '=':
            case '-':
                if (size == 0)
                    return 2;
                return size;
            default:
                if (size == 0)
                    return 1;
                return size;
            }
        }
        if (sym == '/')
        {
            pointer++;
            sym = *pointer;
            /*if(sym != '/')
            {
              if(size == 0) return 1;
              return size;
            } else
            {
              if(size == 0) return 2;
              return size;
            }*/

            switch (sym)
            {
            case '/':
            case '=':
                if (size == 0)
                    return 2;
                return size;
            default:
                if (size == 0)
                    return 1;
                return size;
            }
        }

        // if(IsOperator(pointer)) return size;
        pointer++;
        size++;
    } while (true);
}

} // namespace storm::scripting
