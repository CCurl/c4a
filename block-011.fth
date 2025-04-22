Block #011: case / case! / end-cases / switch

NOTE: you can't use z" or ." when using 'case!'
Also, any case for #0 has to be the 1st case in the table

find switch loaded?

case (n--)v, find drop v, ;
case!(n--)v, here v, 1 state wc! ;
end-cases(--)0 v, 0 v, ;
switch(n cases--)>t >abegin
   a@ t@ @ =ift@ cell+ @ >r atdrop exitthen
   t@ 2 cells + t!
   t@ @if0atdrop exitthen
 again;









