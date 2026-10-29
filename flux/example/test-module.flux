get regex
out "MODULE IMPORTED"
out regex.search("(cat|dog)+", "xxcatdogyy")
out regex.findall("(ab|cd)+", "abab xx cdcd")
out regex.fullmatch("[A-Z][a-z]{2,5}", "Hello")
out regex.group("(\\d+)-(\\w+)", "123-Alex", 1)
out regex.groups("(\\d+)-(\\w+)", "123-Alex")
out regex.search("a+?", "aaaa")
out regex.search("(ab|a)b", "ab")