there are two sections:

escape codes
error/warning messages

there are a a few kinds of bad input that the program won't detect; for
example, it doesn't check what you end a color context with -- for example:

 this will be parsed without complaint (even though it doesn't make sense}.

also, it won't tell you where it found an error when it does find one.

escape codes
~~~~~~~~~~~~

all escape codes are case-sensitive.

\a??
Attribute
sets the character attribute
?? is the new character attribute specified as a pair of hex digits; the
first digit is the new background color and the second digit is the new
foreground color

\b?
Background
sets the character background color
? is the new background color specified as a single hex digit

\f?
Foreground
sets the character foreground color
? is the new foreground color specified as a single hex digit

\l
Last color
switches to the last color on the color stack

\r
Restore color context
removes the last color from the top of the color stack, then switches to the
last color on the color stack

\s
Save color context
puts the current color on the color stack

\x??
arbitrary character code, in heX
writes an arbitrary character
?? is the ASCII code of the character to write in hex

\?
literal symbol
writes a literal symbol without interpreting it
? is the symbol
this is useful when a symbol, such as { (which normally denotes the beginning
of an aside), appears in a context where it shouldn't be interpreted, like
ascii art or a smiley. :{) becomes :\{\), and the \s disappear when the file
is written.

\B
Bold
enters bold mode. semantically similar to the open parenthesis/.., except
thatyou use \r to get out of bold mode, instead of a close parenthesis/...
example:
this is \Bemphasized\r for effect.

\L[action,target][link normal text][link selected text]
Link
creates a hyperlink
if either the action or the target contain spaces or symbols, they should be
"enclosed in quotes". to put a quote character inside quotes, "escape it like
\"this\"". the link text, normal and selected, can contain (almost) all the
escapes that normal text can. color escapes, for sure.

example:

\L[goto,"hbin:about.hbi"][text][text]

error/warning messages
~~~~~~~~~~~~~~~~~~~~~~

 assertion failed ...

probably this means that you forgot a comma or some punctuation in a link.
don't be confused, the line number of the assertion failure is the line
number in MY source code, and isn't really relevant to you. if you can't find
any problems with your escapes, copy the exact text of the message and send
it to me, along with a copy of the file that causes the assertion failure.

 expected hex digit, found...

the translator was looking for a hex digit, but found something else; for
example, if you wrote the escape code \fg, that would cause this warning,
because g is not a hexadecimal digit.

 newline in the middle of link text!

you get this if the line ends before the translator finishes reading in link
text. for example, the following line produces this error, because it is
missing the last ]:

\L[goto,"hbin:about.hbi"][text][text

 popped too many colors off the color stack!

there were too many close parentheses/brackets/braces, or too many \r
(restore color context) escapes. look for \r's without matching \s's, and for
smiley faces.

 you can't have a link within a link, buster!

exactly what it says. example:

\L[goto,"hbin:about.hbi"][text\L[goto,"hbin:about.hbi"][text][text]][text\L[goto,"hbin:about.hbi"][text][text]]

 illegal escape code: ...

that escape code is not in the list.

 ...other common problems

large patches of an unexpected color are probably a stray smiley or
something; if you have missing characters, look for stray \'s (literal \'s
can be made with the escape code \\). if it returns to an incorrect color
after closing a pair of parentheses, brackets, or braces, look for a stray \s
escape or a stray open parenthesis/bracket/brace, maybe in a smiley or
frowny.

