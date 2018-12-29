DEF readint()
  res := 0;
  c := 0;
  WHILE (c < '0') | (c > '9') DO
    c := readchar();
  END;
  WHILE !((c < '0') | (c > '9')) DO
    res := res * 10 + (c - 48);
    c := readchar();
  END;
  RETURN res;
END;

DEF _printint_rec(n)
  IF !n THEN
    RETURN;
  END;
  _printint_rec(n / 10);
  printchar(n % 10 + '0');
END;

DEF printint(n)
  IF !n THEN
    printchar(48);
  ELSE
    _printint_rec(n);
  END;
END;

DEF hanoi(n, a, b)
  IF !n THEN  # comment
    RETURN;
  END;
  hanoi(n - 1, a, 6 - a - b);
  printint(n);
  printchar(' ');
  printint(a);
  printchar(' ');
  printint(b);
  printchar(10);
  hanoi(n - 1, 6 - a - b, b);
END;

DEF main()
  hanoi(readint(), 1, 3);
END;

