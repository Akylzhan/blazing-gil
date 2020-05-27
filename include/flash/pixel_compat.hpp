#include <blaze/Blaze.h>
#include <boost/gil/pixel.hpp>
#include <functional>
#include <type_traits>

template <typename ChannelValue, typename Layout, bool IsRowVector = blaze::rowVector>
struct pixel_vector
    : boost::gil::pixel<ChannelValue, Layout>,
      blaze::DenseVector<pixel_vector<ChannelValue, Layout, IsRowVector>, IsRowVector> {
    using parent_t = boost::gil::pixel<ChannelValue, Layout>;
    using boost::gil::pixel<ChannelValue, Layout>::pixel;

    using This = pixel_vector<ChannelValue, Layout, IsRowVector>;
    using Basetype = blaze::DenseVector<This, IsRowVector>;
    using ResultType = This;

    using TransposeType = pixel_vector<ChannelValue, Layout, !IsRowVector>;

    static constexpr bool simdEnabled = false;

    using ElementType = ChannelValue;
    using TagType = blaze::Group0;
    using ReturnType = const ChannelValue&;
    using CompositeType = const This&;

    using Reference = ChannelValue&;
    using ConstReference = const ChannelValue&;
    using Pointer = ChannelValue*;
    using ConstPointer = const ChannelValue*;

    // TODO: rule of 5
    template <typename OtherChannelValue>
    pixel_vector(pixel_vector<OtherChannelValue, Layout> other) : parent_t(other)
    {
    }

    template <typename OtherChannelValue>
    pixel_vector& operator=(const pixel_vector<OtherChannelValue, Layout> other)
    {
        parent_t::operator=(other);
        return *this;
    }

    template <typename VT, bool TransposeFlag>
    pixel_vector& operator=(const blaze::DenseVector<VT, TransposeFlag>& v)
    {
        if ((~v).size() != size()) {
            throw std::invalid_argument(
                "incoming vector has incompatible size with this pixel vector");
        }

        for (std::size_t i = 0; i < size(); ++i) {
            (*this)[i] = (~v)[i];
        }

        return *this;
    }

  private:
    template <typename OtherChannelValue, typename Op>
    void perform_op(pixel_vector<OtherChannelValue, Layout> p, Op op)
    {
        constexpr auto num_channels = boost::gil::num_channels<parent_t>{};
        auto& current = *this;
        for (std::ptrdiff_t i = 0; i < num_channels; ++i) {
            current[i] = op(current[i], p[i]);
        }
    }

  public:
    std::size_t size() const { return boost::gil::num_channels<parent_t>{}; }

    constexpr bool canAlias() const { return true; }

    template <typename Other>
    bool isAliased(const Other* alias) const noexcept
    {
        return static_cast<const void*>(this) == static_cast<const void*>(alias);
    }

    template <typename Other>
    bool canAlias(const Other* alias) const noexcept
    {
        return static_cast<const void*>(this) == static_cast<const void*>(alias);
    }

    // TODO: Add SFINAE for cases when parent_t::is_mutable is false
    template <typename OtherChannelValue, typename OtherLayout>
    pixel_vector& operator+=(pixel_vector<OtherChannelValue, OtherLayout> other)
    {
        perform_op(other, std::plus<>{});
        return *this;
    }

    template <typename OtherChannelValue, typename OtherLayout>
    pixel_vector& operator-=(pixel_vector<OtherChannelValue, OtherLayout> other)
    {
        perform_op(other, std::minus<>{});
        return *this;
    }

    template <typename OtherChannelValue, typename OtherLayout>
    pixel_vector& operator*=(pixel_vector<OtherChannelValue, OtherLayout> other)
    {
        perform_op(other, std::multiplies<>{});
        return *this;
    }

    template <typename OtherChannelValue, typename OtherLayout>
    pixel_vector& operator/=(pixel_vector<OtherChannelValue, OtherLayout> other)
    {
        perform_op(other, std::divides<>{});
        return *this;
    }
};

template <typename ChannelValue1, typename ChannelValue2, typename Layout, bool IsRowVector>
inline pixel_vector<std::common_type_t<ChannelValue1, ChannelValue2>, Layout>
operator+(pixel_vector<ChannelValue1, Layout, IsRowVector> lhs,
          pixel_vector<ChannelValue2, Layout, IsRowVector> rhs)
{
    lhs += rhs;
    return lhs;
}

template <typename ChannelValue1, typename ChannelValue2, typename Layout, bool IsRowVector>
inline pixel_vector<std::common_type_t<ChannelValue1, ChannelValue2>, Layout>
operator-(pixel_vector<ChannelValue1, Layout, IsRowVector> lhs,
          pixel_vector<ChannelValue2, Layout, IsRowVector> rhs)
{
    lhs -= rhs;
    return lhs;
}

template <typename ChannelValue1, typename ChannelValue2, typename Layout, bool IsRowVector>
inline pixel_vector<std::common_type_t<ChannelValue1, ChannelValue2>, Layout>
operator*(pixel_vector<ChannelValue1, Layout, IsRowVector> lhs,
          pixel_vector<ChannelValue2, Layout, IsRowVector> rhs)
{
    lhs *= rhs;
    return lhs;
}

template <typename ChannelValue1, typename ChannelValue2, typename Layout, bool IsRowVector>
inline pixel_vector<std::common_type_t<ChannelValue1, ChannelValue2>, Layout>
operator/(pixel_vector<ChannelValue1, Layout, IsRowVector> lhs,
          pixel_vector<ChannelValue2, Layout, IsRowVector> rhs)
{
    lhs /= rhs;
    return lhs;
}

template <typename Pixel, bool IsRowVector = blaze::rowVector>
struct pixel_vector_type;

template <typename ChannelValue, typename Layout, bool IsRowVector>
struct pixel_vector_type<boost::gil::pixel<ChannelValue, Layout>, IsRowVector> {
    using type = pixel_vector<ChannelValue, Layout, IsRowVector>;
};

namespace blaze
{
template <typename ChannelValue, typename Layout, bool IsRowVector>
struct UnderlyingElement<pixel_vector<ChannelValue, Layout, IsRowVector>> {
    using Type = ChannelValue;
};

template <typename ChannelValue, typename Layout, bool IsRowVector>
struct IsDenseVector<pixel_vector<ChannelValue, Layout, IsRowVector>> : std::true_type {
};
} // namespace blaze