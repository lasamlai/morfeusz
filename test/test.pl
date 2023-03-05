:- use_module(library(morfeusz)).

:- begin_tests(zażółć_gęślą_jaźń).

:- if(dict_id('')).

test(default_dictionary,
     Y =@= [i(0, 1, 'Zażółć', 'Zażółć', ign),
            i(1, 2, gęślą, gęślą, ign),
            i(2, 3, jaźń, jaźń, ign)]) :-
    morfeusz_analyse("Zażółć gęślą jaźń", Y).

:- else.

test(sgjp_polimorf_dictionary,
     true(permutation(Y, [i(0,1,'Zażółć',zażółcić,impt:sg:sec:perf),
                          i(1,2,gęślą,gęśl,subst:sg:inst:f),
                          i(1,2,gęślą,gęśla,subst:sg:inst:f),
                          i(1,2,gęślą,gęślić,fin:pl:ter:imperf),
                          i(2,3,jaźń,jaźń,subst:sg:nom:f),
                          i(2,3,jaźń,jaźń,subst:sg:acc:f)]))) :-
    morfeusz_analyse("Zażółć gęślą jaźń", Y).

:- endif.

:- end_tests(zażółć_gęślą_jaźń).
