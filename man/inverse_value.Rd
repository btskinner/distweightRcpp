% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RcppExports.R
\name{inverse_value}
\alias{inverse_value}
\title{Compute inverse values from vector}
\usage{
inverse_value(d, exp, transform)
}
\arguments{
\item{d}{Vector of values (e.g., distances)}

\item{exp}{Rate of decay}

\item{transform}{== "log" if natural log transform}
}
\value{
Vector of inverse weights
}
\description{
Compute inverse values from vector
}
