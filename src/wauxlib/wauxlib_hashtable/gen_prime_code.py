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

def lstr(l):
    return map(lambda x: str(x), l)


def prime_enum(primes):
    result = "typedef enum {\n" \
             "EMPTY_LIST,\n" 
    for i in range(len(primes)):
        result += "PRIME%02d,\n" % (i + 1)
    result += "NUM_PRIMES\n} prime_index_t;\n"
    return result

def prime_list(primes):
    result = "static int primes[NUM_PRIMES] = {0, "
    result += ", ".join(lstr(primes)) + "};\n"
    return result

def main():
    primes = gen_primes(186)
    print(primes)
    print(prime_enum(primes))
    print(prime_list(primes))
    print(probe_count(primes))

if __name__ == '__main__':
    main()