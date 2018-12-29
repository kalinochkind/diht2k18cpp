DEF main()
  s := 0;
  WHILE 1 DO
    $s := readchar();
    IF $s = 10 THEN
      s := s - 4;
      BREAK;
    END;
    s := s + 4;
  END;
  FOR i := 0, !(i * 2 > s), 4 DO
    IF $i - $(s - i) THEN
      printchar('n');
      printchar('o');
      RETURN;
    END;
  END;
  printchar('y');
  printchar('e');
  printchar('s');
END;
