just takes bf code and flips left and right chars then prints back
that's it
ends on file end so input needs to be piped

~chars~|charc|input|00|00|01|00|isnotlr|isnotend|00
                           T  F
    reset and setup
    check if left
    if left
        print right
        lower isnotlr flag
    check if right
    if right
        print left
        lower isnotlr flag
    check if end
    if end
        lower isnotend flag
    if isnotlr and isnotend
        print char
    isnotend

++++++++ ++++++++ ++++++++ ++++++++ 
++++++++ ++++++++ ++++++++ ++++ left >
++++++++ ++++++++ ++++++++ ++++++++ 
++++++++ ++++++++ ++++++++ ++++++ right >
++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ 
++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ 
++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ 
++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ ++++++++ +++++++ end >
 

[

]

n1|n2|00|00|01|00
             T  F
+++ n1 >
+++ n2 >
00 >
00 >
+

[<<<-<-[>]>>]
>>[>]<<<<
<[>>+<<[-]]
>[>[-]+<[-]]>
[>]>>
++++++++ ++++++++ ++++++++ ++++++++ 
++++++++ ++++++++ 0
.