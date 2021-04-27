(defn abc [int a] int
    (* a (if 1 42 10)))

(defn iadd [int a int b] int
    (+ a b))

(defn fadd [float a float b] float
    (+ a b))

(defn main [int a] int
    (abc a))
