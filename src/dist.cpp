// dist.cpp
#include <shared.h>
#include <Rcpp.h>

using namespace Rcpp;

//' Compute distance between each coordinate pair (many to many)
//' and return matrix.
//'
//' @param xlon Vector of longitudes for starting coordinate pairs
//' @param xlat Vector of latitudes for starting coordinate pairs
//' @param ylon Vector of longitudes for ending coordinate pairs
//' @param ylat Vector of latitudes for ending coordinate pairs
//' @param funname String name of distance function: Haversine, Vincenty
//' @return Matrix of distances between each coordinate pair in meters
//' @export
// [[Rcpp::export]]
NumericMatrix dist_mtom(const NumericVector& xlon,
			const NumericVector& xlat,
			const NumericVector& ylon,
			const NumericVector& ylat,
			std::string funname) {

  // select function
  XPtr<funcPtr> xpfun = choose_func(funname);
  funcPtr fun = *xpfun;

  int n = xlon.size();
  int k = ylon.size();

  NumericMatrix dist(n,k);

  for(int i = 0; i < n; i++) {
    for(int j = 0; j < k; j++) {

      // compute distance and store
      dist(i,j) = fun(xlon[i], xlat[i], ylon[j], ylat[j]);

    }
  }

  return dist;

}
//' Compute distance between corresponding coordinate pairs in data frame.
//'
//' Compute distance between corresponding coordinate pairs and return vector.
//' For use when creating a new data frame or tbl_df column.
//'
//' @param xlon Vector of longitudes for starting coordinate pairs
//' @param xlat Vector of latitudes for starting coordinate pairs
//' @param ylon Vector of longitudes for ending coordinate pairs
//' @param ylat Vector of latitudes for ending coordinate pairs
//' @param funname String name of distance function: Haversine, Vincenty
//' @return Vector of distances between each coordinate pair in meters
//' @export
// [[Rcpp::export]]
NumericVector dist_df(const NumericVector& xlon,
		      const NumericVector& xlat,
		      const NumericVector& ylon,
		      const NumericVector& ylat,
		      std::string funname) {

  // select function
  XPtr<funcPtr> xpfun = choose_func(funname);
  funcPtr fun = *xpfun;

  int k = ylon.size();
  NumericVector dist(k);

  for(int i = 0; i < k; i++) {

    // compute distance and store
    dist[i] = fun(xlon[i], xlat[i], ylon[i], ylat[i]);

  }

  return dist;

}

//' Compute one to many distances.
//'
//' Compute distances between single starting coordinate and vector of
//' ending coordinates (one to many) and return vector.
//'
//' @param xlon Longitude for starting coordinate pair
//' @param xlat Latitude for starting coordinate pair
//' @param ylon Vector of longitudes for ending coordinate pairs
//' @param ylat Vector of latitudes for ending coordinate pairs
//' @param funname String name of distance function: Haversine, Vincenty
//' @return Vector of distances in meters
//' @export
// [[Rcpp::export]]
NumericVector dist_1tom(const double& xlon,
			const double& xlat,
			const NumericVector& ylon,
			const NumericVector& ylat,
			std::string funname) {

  // select function
  XPtr<funcPtr> xpfun = choose_func(funname);
  funcPtr fun = *xpfun;

  int k = ylon.size();
  NumericVector dist(k);

  for(int i = 0; i < k; i++) {

    // compute distance and store
    dist[i] = fun(xlon, xlat, ylon[i], ylat[i]);

  }

  return dist;

}

//' Compute one to one distance.
//'
//' Compute distance between two points (one to one) and return single value.
//'
//' @param xlon Longitude for starting coordinate pair
//' @param xlat Latitude for starting coordinate pair
//' @param ylon Longitude for ending coordinate pair
//' @param ylat Latitude for ending coordinate pair
//' @param funname String name of distance function: Haversine, Vincenty
//' @return Distance in meters
//' @export
// [[Rcpp::export]]
double dist_1to1(const double& xlon,
		 const double& xlat,
		 const double& ylon,
		 const double& ylat,
		 std::string funname) {

  // select function
  XPtr<funcPtr> xpfun = choose_func(funname);
  funcPtr fun = *xpfun;

  // compute and return
  return fun(xlon, xlat, ylon, ylat);

}

//' Interpolate population/inverse-distance-weighted measures.
//'
//' Interpolate population/inverse-distance-weighted measures for each \strong{x}
//' coordinate using measures taken at surrounding \strong{y} coordinates.
//' Ending measures are double weighted by population and distance so that
//' surrounding measures taken in nearby areas and those with greater
//' populations are given more weight in final average.
//'
//' @param x_df DataFrame with coordinates that need weighted measures
//' @param y_df DataFrame with coordinates at which measures were taken
//' @param measure_col String name of measure column in y_df
//' @param x_id String name of unique identifer column in x_df
//' @param x_lon_col String name of column in x_df with longitude values
//' @param x_lat_col String name of column in x_df with latitude values
//' @param y_lon_col String name of column in y_df with longitude values
//' @param y_lat_col String name of column in y_df with latitude values
//' @param pop_col String name of column in x_df with population values
//' @param dist_function String name of distance function: "Haversine" (default) or
//' "Vincenty"
//' @param dist_transform String value of distance weight transform: "level" (default)
//' or "log"
//' @param decay Numeric value of distance weight decay: 2 (default)
//' @return Dataframe of population/distance-weighted values
//' @export
// [[Rcpp::export]]
DataFrame popdist_weighted_mean(DataFrame x_df,
				DataFrame y_df,
				std::string measure_col,
				std::string x_id = "id",
				std::string x_lon_col = "lon",
				std::string x_lat_col = "lat",
				std::string y_lon_col = "lon",
				std::string y_lat_col = "lat",
				std::string pop_col = "pop",
				std::string dist_function = "Haversine",
				std::string dist_transform = "level",
				double decay = 2) {

  // init
  CharacterVector id = x_df[x_id];
  NumericVector xlon = x_df[x_lon_col];
  NumericVector xlat = x_df[x_lat_col];
  NumericVector meas = y_df[measure_col];
  NumericVector ylon = y_df[y_lon_col];
  NumericVector ylat = y_df[y_lat_col];
  NumericVector popw = y_df[pop_col];

  int n = xlon.size();
  int k = ylon.size();
  NumericVector out(n);

  // loop to compute
  for (int i = 0; i < n; i++) {

    // check for interrupt
    if(i % 1000 == 0)
      Rcpp::checkUserInterrupt();

    // distance vector
    NumericVector dist = dist_1tom(xlon[i], xlat[i], ylon, ylat, dist_function);

    // inverse distance weights
    NumericVector idw = inverse_value(dist, decay, dist_transform);

    // population adjusted idw
    NumericVector w = idw * popw;

    // weight denominator
    double w_sum = 0;
    for (int j = 0; j < k; j++) {
      w_sum += w[j];
    }

    // weight the measure_i
    NumericVector tmp = (w * meas) / (w_sum);

    // sum the wmeasure_i
    double sum = 0;
    for (int j = 0; j < k; j++) {
      sum += tmp[j];
    }

    out[i] = sum;

  }

  return DataFrame::create(_["id"] = id,
			   _["wmeasure"] = out);

}

//' Interpolate inverse-distance-weighted measures.
//'
//' Interpolate inverse-distance-weighted measures for each \strong{x}
//' coordinate using measures taken at surrounding \strong{y} coordinates.
//' Ending measures are weighted by inverse distance so that
//' surrounding measures taken in nearby areas are given more weight in final
//' average.
//'
//' @param x_df DataFrame with coordinates that need weighted measures
//' @param y_df DataFrame with coordinates at which measures were taken
//' @param measure_col String name of measure column in y_df
//' @param x_id String name of unique identifer column in x_df
//' @param x_lon_col String name of column in x_df with longitude values
//' @param x_lat_col String name of column in x_df with latitude values
//' @param y_lon_col String name of column in y_df with longitude values
//' @param y_lat_col String name of column in y_df with latitude values
//' @param dist_function String name of distance function: "Haversine" (default) or
//' "Vincenty"
//' @param dist_transform String value of distance weight transform: "level" (default)
//' or "log"
//' @param decay Numeric value of distance weight decay: 2 (default)
//' @return Dataframe of distance-weighted values
//' @export
// [[Rcpp::export]]
DataFrame dist_weighted_mean(DataFrame x_df,
			     DataFrame y_df,
			     std::string measure_col,
			     std::string x_id = "id",
			     std::string x_lon_col = "lon",
			     std::string x_lat_col = "lat",
			     std::string y_lon_col = "lon",
			     std::string y_lat_col = "lat",
			     std::string dist_function = "Haversine",
			     std::string dist_transform = "level",
			     double decay = 2) {

  // init
  CharacterVector id = x_df[x_id];
  NumericVector xlon = x_df[x_lon_col];
  NumericVector xlat = x_df[x_lat_col];
  NumericVector meas = y_df[measure_col];
  NumericVector ylon = y_df[y_lon_col];
  NumericVector ylat = y_df[y_lat_col];

  int n = xlon.size();
  int k = ylon.size();
  NumericVector out(n);

  // loop to compute
  for (int i = 0; i < n; i++) {

    // check for interrupt
    if(i % 100 == 0)
      Rcpp::checkUserInterrupt();

    // distance vector
    NumericVector dist = dist_1tom(xlon[i], xlat[i], ylon, ylat, dist_function);

    // inverse distance weights
    NumericVector w = inverse_value(dist, decay, dist_transform);

    // weight denominator
    double w_sum = 0;
    for (int j = 0; j < k; j++) {
      w_sum += w[j];
    }

    // weight the measure_i
    NumericVector tmp = (w * meas) / (w_sum);

    // sum the wmeasure_i
    double sum = 0;
    for (int j = 0; j < k; j++) {
      sum += tmp[j];
    }

    out[i] = sum;

  }

  return DataFrame::create(_["id"] = id,
			   _["wmeasure"] = out);

}

//' Find minimum distance.
//'
//' Find minimum distance between each starting point in \strong{x} and
//' possible end points, \strong{y}.
//'
//' @param x_df DataFrame with coordinates that need weighted measures
//' @param y_df DataFrame with coordinates at which measures were taken
//' @param x_id String name of unique identifer column in x_df
//' @param x_lon_col String name of column in x_df with longitude values
//' @param x_lat_col String name of column in x_df with latitude values
//' @param y_lon_col String name of column in y_df with longitude values
//' @param y_lat_col String name of column in y_df with latitude values
//' @param dist_function String name of distance function: "Haversine" (default) or
//' "Vincenty"
//' @return DataFrame with minimum distance in meters
//' @export
// [[Rcpp::export]]
DataFrame dist_min(DataFrame x_df,
		   DataFrame y_df,
		   std::string x_id = "id",
		   std::string x_lon_col = "lon",
		   std::string x_lat_col = "lat",
		   std::string y_lon_col = "lon",
		   std::string y_lat_col = "lat",
		   std::string dist_function = "Haversine") {

   // init
  CharacterVector id = x_df[x_id];
  NumericVector xlon = x_df[x_lon_col];
  NumericVector xlat = x_df[x_lat_col];
  NumericVector ylon = y_df[y_lon_col];
  NumericVector ylat = y_df[y_lat_col];

  int n = xlon.size();
  NumericVector out(n);

  // loop
  for (int i = 0; i < n; i++) {

    // check for interrupt
    if(i % 100 == 0)
      Rcpp::checkUserInterrupt();

    // distance vector
    NumericVector dist = dist_1tom(xlon[i], xlat[i], ylon, ylat, dist_function);

    // add min to out
    out[i] = min(dist);

  }

  return DataFrame::create(_["id"] = id,
			   _["mindist"] = out);

}

