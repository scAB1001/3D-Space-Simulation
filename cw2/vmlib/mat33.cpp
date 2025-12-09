#include "mat33.hpp"
#include "mat44.hpp"

Mat33f mat44_to_mat33(Mat44f const &aM) noexcept
{
	Mat33f ret;
	for (std::size_t i = 0; i < 3; ++i)
	{
		for (std::size_t j = 0; j < 3; ++j)
			ret[i, j] = aM[i, j];
	}
	return ret;
}

Mat33f make_uniform_normal(Mat44f const &aM) noexcept
{
	// Mat33f normalMatrix_arrows = mat44_to_mat33(transpose(invert(model2world_arrows)));
	return mat44_to_mat33(transpose(invert(aM)));
}