class num2bin:

    def sf16_bin16(n: float):
        try:
            if n < -6.5536:  n = -6.5536
            elif n > 6.5534: n = 6.5534
            ret = int(round(n * 5000))
        except ValueError:
            ret = 0
        return ret



