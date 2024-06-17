#include <iostream>
#include <math.h>

double area(int N)
{
    double s = 0;
    double height1{};
    double height2{};
    for (int i = 0; i < N; ++i)
    {
        height1 = sqrt(4.0 - pow((i*(2.0/N)), 2));
        height2 = sqrt(4.0 - pow(((i+1)*(2.0/N)), 2));
        s = s + (height1 + height2)/2* (2.0 / N);
    }
    return s;
}


int main()
{
    int N = 100000;
    double s = area(N);
    std::cout << s << std::endl;
}