#include "utils.h"

#if defined(OPENMP_FOUND)
#include <omp.h>
#else
#include <ctime>
#endif

#include <algorithm> // for std::random_shuffle

std::ostream&
operator<<(std::ostream& os, const Direction& direction)
{
    static const std::string direction_name[5] = {"Stay", "North", "South", "East", "West"};
    return os << direction_name[static_cast<int>(direction)];
}

double
get_double_time()
{
#if defined(OPENMP_FOUND)
    return omp_get_wtime();
#else
    timespec foo;
    clock_gettime(CLOCK_REALTIME, &foo);

    return foo.tv_sec + foo.tv_nsec*1e-9;
#endif
}

clock_it::clock_it(const double& delta) :
    delta(delta)
{
}

std::ostream&
operator<<(std::ostream& os, const clock_it& clock_it)
{
    if (clock_it.delta < 1e-3)
    {
        const int us=(clock_it.delta*1e6);
        return os << us << "us";
    }

    const int ms(clock_it.delta*1e3);
    return os << ms << "ms";
}

OmpFlag::OmpFlag(const bool& initial_state) :
    state(initial_state)
{
}

void
OmpFlag::set()
{
//#if defined(OPENMP_FOUND)
//    #pragma omp atomic write
//#endif
    state = true;
}

void
OmpFlag::reset()
{
//#if defined(OPENMP_FOUND)
//    #pragma omp atomic write
//#endif
    state = false;
}

bool
OmpFlag::test() const
{
//#if defined(OPENMP_FOUND)
//    bool read_state;
//    #pragma omp atomic read
//    read_state = state;
//    return read_state;
//#else
    return state;
//#endif
}


static
void
called_function(const int& kk)
{
    const int third = rand();
#if defined(OPENMP_FOUND)
    #pragma omp critical
    std::cout << "   funct " << omp_get_thread_num() << " " << omp_get_level() << " " << third << " " << kk << std::endl;
#endif
}

int
test_random_generator()
{
#if !defined(NDEBUG)

    std::cout << std::hex;

    {
        const int first = rand();
        const int second = rand();

        std::cout << first << " " << second << std::endl;
    }

    {
        int random_buffer[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::random_shuffle(random_buffer, random_buffer+10);
        for (size_t kk=0; kk<10; kk++)
            std::cout << random_buffer[kk] << " ";
        std::cout << std::endl;
    }

    int total_good = 0;
    int total_bad = 0;
    #pragma omp parallel default(none) shared(std::cout, total_good, total_bad)
    {
        const int first = rand();
        #pragma omp critical
        std::cout << "thread " << omp_get_thread_num() << "/" << omp_get_team_size(1) << " " << omp_get_level() << " " << first << std::endl;
        #pragma omp for
        for (size_t kk=0; kk<10; kk++)
        {
            const int second = rand();
            #pragma omp critical
            std::cout << "  in for " << omp_get_thread_num() << " " << omp_get_level() << " " << second << " " << kk << std::endl;
            called_function(kk);
            #pragma omp atomic update
            total_good++;
            total_bad++;
        }
    }

    std::cout << std::dec;
    std::cout << "good " << total_good << " bad " << total_bad << std::endl;

#endif
    return rand();
}

static
void
test_random_throughtput()
{
    const int payload = 2000000;
    { // rand no parallel
        const double start_time = get_double_time();
        for (int kk=0; kk<payload; kk++)
        {
            rand();
        }
        const double end_time = get_double_time();

        std::cout << "rand no parallel " << static_cast<int>(1e-3*payload/(end_time-start_time)) << "krand/s " << clock_it(end_time-start_time) << std::endl;
    }

#if defined(OPENMP_FOUND)
    { // rand parallel
        const double start_time = get_double_time();
#pragma omp parallel for default(none)
        for (int kk=0; kk<payload; kk++)
        {
            rand();
        }
        const double end_time = get_double_time();

        std::cout << "rand parallel " << static_cast<int>(1e-3*payload/(end_time-start_time)) << "krand/s " << clock_it(end_time-start_time) << std::endl;
    }
#endif

    { // boost::mt no parallel
        Rng gen;
        gen.seed(rand());

        const double start_time = get_double_time();
        for (int kk=0; kk<payload; kk++)
        {
            gen();
        }
        const double end_time = get_double_time();

        std::cout << "boost::mt no parallel " << static_cast<int>(1e-3*payload/(end_time-start_time)) << "krand/s " << clock_it(end_time-start_time) << std::endl;
    }

#if defined(OPENMP_FOUND)
    { // boost::mt parallel
        Rng gen_common;
        gen_common.seed(rand());

        const double start_time = get_double_time();
        Rng gen_thread;
#pragma omp parallel default(none) shared(gen_common, std::cout) private(gen_thread)
        {
#pragma omp critical
            {
                gen_thread.seed(gen_common);
                //std::cout << "** " << gen_thread() << " " << omp_get_thread_num() << std::endl;
            }

#pragma omp for schedule(static, 10000)
            for (int kk=0; kk<payload; kk++)
            {
                gen_thread();
            }
        }
        const double end_time = get_double_time();

        std::cout << "boost::mt parallel " << static_cast<int>(1e-3*payload/(end_time-start_time)) << "krand/s " << clock_it(end_time-start_time) << std::endl;
    }
#endif

}

void
test_random()
{   // random test
    std::cout << "Doing random test...\n";
    const int rand_aa = test_random_generator();
    srand(1);
    const int rand_bb = test_random_generator();
    srand(time(NULL));
    const int rand_cc = test_random_generator();
    std::cout << std::hex;
    std::cout << rand_aa << std::endl << rand_bb << std::endl << rand_cc << std::endl;
    std::cout << std::dec;
    assert( rand_aa == rand_bb );
    test_random_throughtput();
    std::cout << "...done!\n";
}
