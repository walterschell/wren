import math
def is_prime(p, primes):
    if p == 2:
        return True
    for i in primes:
        if p % i == 0:
            return False
        if i > math.sqrt(p):
            return True
    return True

def gen_primes(num_primes):
    p = 3
    result = [2]
    while len(result) < num_primes and p < 2**32:
        if is_prime(p, result):
            try: 
                if len(result) == 0 or p >= 2*result[len(result)-1]:
                    result.append(p)
                    print("List Size: %d Prime: %d" % (len(result), p))
                    p *= 2
                    p -= 1
            except:
                import pdb; pdb.set_trace()
        p += 2
    return result

def main():
    primes = gen_primes(186)
    print(primes)

if __name__ == '__main__':
    main()