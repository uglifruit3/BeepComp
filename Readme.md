# BeepComp
### A Beep Compiler for making PC speaker music on Linux
-----------------
## Table of Contents
1. [About](https://github.com/uglifruit3/BeepComp#about)
2. [Installation](https://github.com/uglifruit3/BeepComp#installation)
3. [Usage](https://github.com/uglifruit3/BeepComp#usage)
   1. [Invocation](https://github.com/uglifruit3/BeepComp#invocation)
   2. [Notation and Syntax](https://github.com/uglifruit3/BeepComp#notation-and-syntax)
4. [Example](https://github.com/uglifruit3/BeepComp#example)
5. [Limitations and To-Do](https://github.com/uglifruit3/BeepComp#limitations-and-to-do)

## About
-----------------
BeepComp is a command line tool for converting a basic plain text, human-readable form of musical notation into a shell script which plays music on a PC speaker using the `beep` utility. 

It aims to alleviate the difficulties involved in creating melodies with `beep` alone and includes a few tools for making them more complex than would otherwise be convenient.

This is an amateur project; having undertaken it with no notable prior experience in programming, I gratefully welcome any constructive criticism or suggestions.

## Installation
-----------------
The included makefile performs all actions necessary for installation. Clone the repository to your machine, navigate to it, and run 
```
# make install
```
Naturally, one should also have `beep` installed for BeepComp to be of much use.

## Usage
-----------------
## Invocation
Beepcomp is invoked as shown:
```
beepcomp [-h] [-f infile] [-o outfile]
```
It optionally accepts a plain text input file to read from and an output file to which the shell script can be written. If these are not specified, the program will default to stdin and stdout respectively. This usage information can be displayed using the -h flag.

* In stdin mode, the program will accept input line-by-line, reporting errors as they occur. It will stop taking output once EOF is specified. Syntax errors will not cause the program to exit.
* In stdout mode, the program will output the shell script to the terminal upon reaching EOF.
* When given an input file to read from, the program will exit upon encountering any syntax errors after informing the user of such. Output will not be written.
* When given an output file, the program will automatically make the file executable by the user. Note that it will overwrite an existing output file, so one should use in/out redirection in the shell if they intend to append BeepComp's output.

## Notation and Syntax
### Writing Notes
BeepComp interprets a note as a continuous string with the structure 
```
<note name>[accidental]<octave>[duration][effects macro]
``` 
The square-braced items are optional for producing valid notes. Notes are required to be separated by spaces.
* Note name - the letters 'A' through 'G' are valid note names.
* Accidental - for specifying a sharp, flat, or natural (for negating accidentals given by key signatures).

character | meaning
----------|--------
 \#        | sharp
 b        | flat
 n        | natural

* Octave - specifies the octave at which a note will be played. The numbers 1-9 are valid syntax. Each octave starts with the note C and ends with B. So a third-octave B and fourth-octave C are one half-step apart from each other.
* Duration - specifies the number of beats for which a note will last. These follow the conventions of regular musical notation, with the characters below corresponding to certain musical counterparts:

character |  note type
----------|-----------
 o        | whole note
 ,        | half note
 ^        | eigth note
 .        | dot

  * If no duration character is given, the note is taken to be a quarter note. The eighth note characters can be repeated for further subdivisions (e.g. '^^' gives a sixteenth note, '^^^' gives a thirty-second note, and so on). The dot can be used in a similar manner. **Note:** dots must be the final duration character(s) if multiple are given. 
* Effects macros will be discussed in their respective section.

A few small examples may be helpful.
* To convert an A for one beat into a compatible `beep` command, one writes `A4` to produce an A in the fourth octave.
* BeepComp would recognize `Bb4^^` as a B-flat in the fourth octave lasting the duration of a sixteenth note.
* `Eb5,.` produces a dotted half note, with the pitch of a fifth-octave E flat
* Notes are delimited by spaces, so to play the first, third, and fifth of a C-major chord one writes `C4 E4 G4`.

### Timing Elements
BeepComp also accepts three non-note elements to aid in designing more complex rhythms.
* Rests
  * Rests will silence the speaker for a specified duration and are represented with the 'r' character. Once a rest is placed, its duration is specified in the same manner that notes are (`r` gives a quarter rest, `r^^` a sixteenth, etc.).
* Ties
  * Ties combine the durations of adjacent notes with the same pitch and are represented with the hyphen ('-') character placed between two tied notes. 
  * For example, `G#5 - G#5` makes two quarter note G-sharps a singular half note G-sharp.
* Parentheses
  * Notes may be surrounded by a pair of parentheses, with the open parenthesis prefaced with a set of eigth note characters that specify some duration. Upon compilation, BeepComp will apply that set of eigth note characters to the contained notes' durations.
  * For example, `^^( A3 C4 E4^ G4 )` is equivalent to `A3^^ C4^^ E4^^^ E4^^`.
  * Parentheses may be nested. So, `^( A4 ^( E5 ) A4 )` expands to `A4^ E5^^ A4^`.

**Note:** rests, ties, and parentheses must be surrounded by at least one space to avoid being interpreted as elements of a note. So, `^^(A4 A5)` and `A4-A4` are invalid syntax and will produce errors. They should instead be written as `^^( A4 A5 )` and `A4 - A4` respectively.

### Defaults and Commands
BeepComp uses a default tempo of 90 beats per minute when determing the duration of notes. The default key signature is C Major, meaning that there will be no changes to the pitches of any notes. These can both be altered at any point, and as many times as necessary, using commands.

A command is invoked using the `set` keyword, followed by the name of the command and the arguments it accepts. **Each command must reside on its own line.** BeepComp recognizes the following commands:

 command |        argument 
---------|-----------------------
  tempo  | new tempo in bpm
   key   | new key 
 arprate | new arpeggiation rate

* For example, to change the tempo to 60 bpm, one would write `set tempo 60`.

BeepComp interprets keys as a note name, followed by a space and either an 'M' for major or 'm' for minor. 
In a key other than C M/A m, BeepComp will automatically alter the pitches of notes in a key upon compilation. For example, if the key has been set to C# M and BeepComp encounters the note `A5`, it will be interpreted as `Ab5` in keeping with the key signature. This can be negated in the same way as in regular musical notation by using the natural accidental. 

* For example, the key of A minor is expressed as "A m"; C sharp major is expressed as "C# M". 
* To change the key signature to C sharp major, one writes `set key C# M`.
* If one intends for `A5` to in fact remain `A5` in the key of C# M, one would write `A5n`.

The `arprate` command will be addressed in the next section. 

### Effects Macros
BeepComp supports an arpeggiation macro to simulate polyphony on `beep` by producing rapid, three-note arpeggios. The macro is invoked using the square braces ('[' and ']') at the end of a note. The note preceding the braces will serve as the first note in the arpeggio. The braces must contain two single-character parameters; the first being the interval in half steps between the original arpeggio note and the next; the second being the interval in half steps between the original note and the final note in the arpeggio. Each parameter is expressed in hexadecimal, meaning numbers between 0 and 15 are accepted in the forms of '0-9' and 'A-F' (A through F must be capitalized). 

* For example, the string `C4[47]` will produce a rapid arpeggio between C4, E4 (four half-steps above C4), and G4 (seven half-steps above C4), lasting the length of a quarter note. One could produce the same arpeggio for half that duration by writing `C4^[47]`.

BeepComp expands an arpeggio to play at a rate of 60 tones per second by default. This can be adjusted using the `arprate` command mentioned above and passing the new arpeggiation rate as the argument.

The only effects macro currently supported is arpeggiation. See Limitations for an explanation of why.

### Comments
BeepComp notation includes a means to make comments for helping with song structure/organization. The percent ('%') character is the reserved comment character, and BeepComp will ignore all input on a line after it is detected. BeepComp also ignores blank lines.

## Example
I put together a little example of what BeepComp can do, using the first verse of Jonathan Coulton's "Still Alive" from the computer game *Portal* in hopes that it will be illustrative of proper syntax/command usage. You can also download and listen to the shell script which BeepComp creates from it, provided you have `beep` installed on your machine. Both files are found in the still\_alive directory.
```
% A short cover of 'Still Alive' from Portal.
set tempo 120
set key D M

^( G5^ r^ F5^ r^ E5 E5 ) F5, % this was a triumph
r, r r^
^( A4 G5^ r^ F5^ r^ E5 E5 - E5 ) F5. % I'm making a note here
D5 E5^ A4^ - A4, r. % HUGE SUCCESS
A4^ E5 F5^ G5^ - G5 ^( E5 C5 - C5 ) % it's hard to overstate
D5. E5 A4^ A4 F5. % my satisfaction

^( B2 D3 F3 D3 A2 D3 F3 D3 ) % bass line

^( G5 F5 E5 E5 ) F5 % Aperture Science
^( F3 D3 B2 D3 F3 D3 A2 D3 F3 ) % (bass)
^( A4 G5 F5 E5 E5 - ) E5 % we do what we must
F5^ D5^ - D5 E5^ A4^ - A4^ % because we can
^( D3 F3 D3 B2 D3 F3 D3 ) % (bass)

E5 F5^ G5^ - G5 ^( E5 C5 - ) C5 % for the good of all
^( D5 E5 - E5 A4 D5 E5 ) % of us, except the
set key F M % (key change)
^( F5 E5 D5 C5 ) r % ones who are dead

A4^ B4^ % but there's 
set arprate 60
F4[70] A4[38] % no sense
set arprate 50
^( G4[48] G4[46] G4[26] G4[24] ) % crying over 
F4^[59] F4^[57] % every
set arprate 45
F4[57] F4[47] % mistake
A4^ B4^ % you just
set arprate 50
F4[47] A4[38] % keep on 
^( C5[47] C5[45] G4[48] G4[46] ) % trying til you
^( F4[59] F4[5B] ) F4[5C] A4[38] % run out of cake

G5^ A5^ % and the 
D5^[58] D5^[38] D5[37] C5[47] F5^ G5^ % science gets done and you
set arprate 45
C5^[59] C5^[49] C5[47] A4[58] D5^ C5^ % make a neat gun for the
set arprate 50
^( F4[59] F4[5C] F4[5C] ) A4[47] % people who are
set arprate 40
A4[47] A4^[49] Gb5, % still alive
```

## Limitations and To-Do
* Portamento and vibrato effects macros were originally envisioned to accompany arpeggiation. However, these effects are based on bending the pitch of a note, and `beep` does not allow smooth transitions between frequencies. From what I can tell, there is something somewhere in software that turns off the PC speaker between tones, rather than allowing it to transition between frequencies while remaining turned on. This makes any attempt at rapidly and smoothly switching tones sound like a saw running and is not worth using for much of anything. Any edifying clarifications on how interactions between the operating system and speaker hardware are handled at a low level are greatly appreciated. Working out smooth transitions between tones--either with `beep`, or something else entirely--would open a great deal of possibilities for making music on the PC speaker.
* To do:
  * Improve the quality and accuracy of error reporting messages.
  * Create a feature that allows users to define their own macros consisting of legal note syntax.
  * Create a custom arpeggio feature that allows users to define arpeggios with less or greater than three notes that may go beneath the first one.
  * Investigate other means of beeping the PC speaker, in pursuit of eventually being able to transition between pitches and implement portamento/vibrato effects.
