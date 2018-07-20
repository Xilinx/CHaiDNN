#include <string>
#include <iostream>

int xiEval (std::string &layerspec_csv);

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "\n[ERROR] Usage : " << argv[0] << " <csv file> " << std::endl << std::endl;
        return -1;
    }
    std::string csvfile (argv[1]);
    //# Run performance eval
    int ret = xiEval (csvfile);

    return ret;
}
