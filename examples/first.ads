;;(+ 1 (+ 2 3)) ;; this is a comment
;;(+ 4 (+ 5 6))

(defn iadd [int a int b] int
    (+ a b))

(defn fadd [float a float b] float
    (+ a b))

(defn main [] int
    (fadd 40.1 1.9)
    (iadd 40 2))


;;(main)

;;(fadd [float a, float b] float
;;    (abc a b))
