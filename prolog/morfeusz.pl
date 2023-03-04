% Prolog interface to the morphological analyzer Morfeusz
%
% Author: Marcin Woliński
% This file is in the public domain.
:- module(morfeusz, [
	      morfeusz/4,
	      morfeusz_analyse/2
	  ]).

% this defines morfeusz_analyse/2:
:- use_foreign_library(foreign(morfeusz2_swipl)).

%!	morfeusz(+String, +Predicate, Start, Stop)
%
%	redefines predicate Predicate/5: Predicate(from,to,form,base,tag)

morfeusz(String, _, _, _) :-
	var(String),
	!,
	fail.
morfeusz(_, Predicate, _, _) :-
	not(atom(Predicate)),
	!,
	fail.
morfeusz(String, Predicate, Start, Stop) :-
	morfeusz_analyse(String, MO),
	abolish(Predicate/5),
	budujgrafmorf(Predicate, Start, Stop, MO).

budujgrafmorf(_, 0, 0, []).
budujgrafmorf(Pred, Start, Stop, [i(Start, Stop, F, H, I)]) :- !,
	T =.. [Pred, Start, Stop, F, H, I],
	assertz(T).
budujgrafmorf(Pred, Start, Stop, [i(Start, K, F, H, I) | MOO]) :-
	T =.. [Pred, Start, K, F, H, I],
	assertz(T),
	budujgrafmorf(Pred, _, Stop, MOO).

%%% Local Variables:
%%% coding: utf-8
%%% mode: prolog
%%% End:
