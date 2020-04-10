#include <blaze/math/dense/DynamicMatrix.h>
#include <blaze/math/expressions/MatMapExpr.h>
#include <blaze/math/dense/StaticVector.h>
#include <boost/gil/image.hpp>
#include <boost/gil/image_view.hpp>
#include <boost/gil/typedefs.hpp>
#include <boost/gil/extension/io/png.hpp>

#include <core.hpp>
#include <iostream>

int main()
{
    blaze::DynamicMatrix<unsigned char> matrix(16, 16, 255);
    auto padded = flash::pad(matrix, 8, 0);

    auto image = flash::to_gray8_image(padded);
    boost::gil::write_view("output.png", boost::gil::view(image), boost::gil::png_tag{});

    std::cout << padded.rows() << ' ' << padded.columns() << '\n';
    for (std::size_t i = 0; i < padded.rows(); ++i)
    {
        for (std::size_t j = 0; j < padded.columns(); ++j)
        {
            std::cout << static_cast<int>(padded(i, j)) << ' ';
        }
        std::cout << '\n';
    }
}