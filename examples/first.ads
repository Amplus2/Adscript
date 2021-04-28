(defn abc [int a] int)

(defn main [int a] i64)

(defn iadd [int a int b] int
    (+ a b))

(defn fadd [float a float b] float
    (+ a b))

(defn main [int a] i64
    (long (int (double (float 10.0))))
    (i64 (abc a)))

(defn abc [int a] int
    (* a (if 1 42 10)))
