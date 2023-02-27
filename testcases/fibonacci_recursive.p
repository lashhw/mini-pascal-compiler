program fibonacci_recursive(output);
VAR a:integer;

FUNCTION fa (a:integer) :integer;
begin
         if a = 1 then
        fa := 1
         else  if a = 0 then
        fa := 0
     else   fa:=fa(a-1)+fa(a-2)
end;


begin
    a:=readlnI;
    writelnI(fa(a))
end.
