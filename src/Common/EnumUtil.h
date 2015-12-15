#ifndef ENUMUTIL
#define ENUMUTIL

#define DEFINE_ENUM_FLAGS(X)                            \
inline X operator|(const X& A, const X& B) {            \
    return (X) ((int) A | (int) B);                     \
}                                                       \
inline void operator|= (X& A, X& B) {                   \
    A = A | B;                                          \
}                                                       \
inline X operator|(const X& A, const int B) {           \
    return (X) ((int) A | B);                           \
}                                                       \
inline void operator|= (X& A, int B) {                  \
    A = A | B;                                          \
}                                                       \
inline X operator|(const X& A, const unsigned int B) {  \
    return (X) ((int) A | B);                           \
}                                                       \
inline void operator|= (X& A, unsigned int B) {         \
    A = A | B;                                          \
}                                                       \
inline X operator&(const X& A, const X& B) {            \
    return (X) ((int) A & (int) B);                     \
}                                                       \
inline void operator&= (X& A, X& B) {                   \
    A = A & B;                                          \
}                                                       \
inline X operator&(const X& A, const int B) {           \
    return (X) ((int) A & B);                           \
}                                                       \
inline void operator&= (X& A, int B) {                  \
    A = A & B;                                          \
}                                                       \
inline X operator&(const X& A, const unsigned int B) {  \
    return (X) ((int) A & B);                           \
}                                                       \
inline void operator&= (X& A, unsigned int B) {         \
    A = A & B;                                          \
}

#endif // ENUMUTIL

