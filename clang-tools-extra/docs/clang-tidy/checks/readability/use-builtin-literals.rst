.. title:: clang-tidy - readability-use-builtin-literals

readability-use-builtin-literals
================================

Finds literals explicitly casted to a type that could be expressed using builtin prefixes or suffixes.

.. code-block: c++

    (char16_t)U'a'; // -> u'a'
    (char)'a'; // -> 'a'
    static_cast<unsigned int>(0x1ul); // -> 0x1u
    reinterpret_cast<long long int>(3ll); // -> 3LL
    float(2.); // -> 2.f
    double{2.}; // -> 2.

Defined for character, integer and floating literals. Removes any suffixes or prefixes before applying the one that corresponds to the type of the cast. Long values will always have uppercase ``L`` recommended, conforming to ``readability-uppercase-literal-suffix``.

In elementary cases, provides a fix-it hint. An explicit cast within a macro will be matched, but will only yield a suggestion for a manual fix.

.. code-block: c++

    #define OPSHIFT ((unsigned)27)
    OPSHIFT; // warning: use builtin 'u' instead of cast to 'unsigned int'

Otherwise, if either the destination type or the literal was substituted from a macro, no warning will be produced.

.. code-block: c++

    #define INT_MAX 2147483647
    static_cast<unsigned>(INT_MAX); // no warning

    #define INTTYPE unsigned
    (INTTYPE)31; // no warning

Explicit casts within templates will also be ignored.
