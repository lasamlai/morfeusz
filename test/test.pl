:- use_module(library(morfeusz)).

:- begin_tests(morfeusz_analyse).

test(zażółć_gęślą_jaźń, Y =@= [i(0, 1, 'Zażółć', 'Zażółć', ign), i(1, 2, gęślą, gęślą, ign), i(2, 3, jaźń, jaźń, ign)]) :-
    morfeusz_analyse("Zażółć gęślą jaźń", Y).

:- end_tests(morfeusz_analyse).
