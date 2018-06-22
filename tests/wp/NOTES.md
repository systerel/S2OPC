# Notes about using FramaC

This is an informal collection of notes that I took while using FramaC

## General requirements about the code

FramaC works better on code that does minimal dynamic memory allocation, and
that is split in small function. Furthermore, PO using \separation clauses grow
exponentially with the number of pointers passed in argument. In those case,
the default time-out value can be too short. To change its value, use the
"-wp-timeout" argument. It seems that Alt-Ergo for now has trouble with those
POs, and the use of external provers can help prove some of them.

## Setting up FramaC

FramaC and Alt-Ergo prover has been installed with opam. Particularly, Alt-Ergo
1.30 version has been installed manually. Other external provers can be used 
with the Why3 platform, and CVC4 1.6 is been used to prove certain POs that are
too big for Alt-Ergo. Why3 0.88.3 has also been installed with opam, and CVC4
has been installed just by taking the compiled binairy and puting it in a bin
folder.
To configure Why3 to detect external prover, run:
"why3 config --detect"

## Using FramaC

FramaC can be invoked with many options. The ones used in this project are :
-wp -wp-rte
-wp-timeout
-wp-prover
-cpp-command
-wp and -wp-rte : For using WP to prove the file and automatically generating
Run-Time Error POs
-wp-timeout : Set the timeout for the proof of one PO. In this project : '=10'
-wp-prover : Use the specified class of prover, the binairies for external
provers must be in a bin folder in the PATH, and why3 has to be configured and
have detected it beforehand. In this project : 'alt-ergo,CVC4'
-cpp-command : To specify all the header files needed to link to the c file.
In this project : '"gcc -C -E -I headerfile1.h -I headerdfile2.h ..."'

An optional command is to use -wp-fct to only prove the specified function name

## Annotating the code with FramaC

###Problèmes rencontrés

####`malloc`
A la présente version de Frama-c (17 - Chlorine), beaucoup de built-in traitant
de l'allocation dynamique ne sont pas encore implémentés, comme `\fresh`,
`\allocable`, `\freeable`...
Par conséquence, le contrat de `malloc` donné par Frama-c dans la libc n'est
pas prouvable. Dans un premier temps, j'ai réecris un stub de `malloc` avec un
contrat simplifié pour rendre la preuve des fonctions appelantes possibles.
Cependant, cette méthode n'était pas satisfaisante car le comportement de
malloc n'était pas déterministe, et que j'avais besoin de savoir si le
pointeur alloué était bien différent de `NULL` à la fin pour prouver des
contrats. Dans un second temps, j'ai écris une fonction static pour extraire
l'opération de `malloc` de la fonction, puis rendu cette fonction invisible à
Frama-c et enfin écrit une definition de la fonction avec un contrat qui est
donc donné comme fait et non plus à prouver. Dans ce contrat, j'ai créé une
variable `ghost bool has_mem` et rends deux résultat en fonction de cette
variable. Cela permet de déterminiser le comportement de malloc en amont de
l'appelant, et ainsi décrire tout les résultats possibles dans l'appelant
directement.

####`assigns` avec `malloc`
La clause assigns prend en compte les cases mémoires définit dans le pre-state
par défaut. Dans une fonction qui alloue de la mémoire, la clause assigns ne
peut pas voir les cases qui sont créées après, sauf si `\at( ... , Post)` est
utilisé.

####`loop assigns`
Ne pas oublier le compteur de boucle qui est assigné.

####"Duplication" de nom de variable dans la preuve du assigns
Pour prouver un `assigns` juste, parfois il faut écrire un contrat
intermédiaire dans le corps de la fonction pour relier les deux variables pour
Frama-c.

####`malloc` d'une variable locale
Donne trop de problèmes a `assigns` pour le prouver. Une solution est d'allouer
directement le pointeur de retour, sans passer par une variable locale.

####`\separated`
Dans une fonction qui manipule les valeurs de plusieurs pointeurs en même
temps, ne pas oublier de préciser que tout les pointeurs pointent bien vers des
objets séparés, même s'ils ne sont pas de même type, avec la clause
`\separated( ... )`.
 
####`assert.h`
Frama-c utilise sa propre fonction `assert` dans les commentaires ACSL qui
malheureusement n'est pas compatible avec la fonction du même nom en C. Pour
résoudre ce problème, j'inclus un header avec une redéfinition de `assert` si
c'est Frama-c qui parcourt le fichier pour le rendre invisible à ses yeux.


