#*******************
# Functions
#*******************

calculate_chebyshev_coefficients <- function(cutoff_frequency, is_high_pass, percent_ripple, number_of_poles, pole_pair)
# This function has been derived from "The Scientist and Engineer's
# Guide to Digital Signal Processing." (http://www.dspguide.com/)
# under the following copyright notice: "All these programs may be
# copied, distributed, and used for any noncommercial purpose."
#
# cutoff_frequency:  cutoff frequency (0.0 to 0.5)
# is_high_pass:  false --> low pass filter, true --> high pass filter
# percent_ripple:  percent ripple (0 to 29)
# number_of_poles:  number of poles (2,4,...20)
# pole_pair:  pole pair (number_of_poles/2)
{
  # calculate the pole location on the unit circle
  RP <- -cos((pole_pair - 0.5) * pi / number_of_poles)
  IP <- sin((pole_pair - 0.5) * pi / number_of_poles)

  # warp from a circle to an ellipse
  if (percent_ripple > 0)
  {
    ES <- sqrt((100 / (100 - percent_ripple)) ** 2 - 1)
    VX <- (1 / number_of_poles) * log((1 / ES) + sqrt((1 / ES ** 2) + 1))
    KX <- (1 / number_of_poles) * log((1 / ES) + sqrt((1 / ES ** 2) - 1))
    KX <- (exp(KX) + exp(-KX)) / 2
    RP <- RP * ((exp(VX) - exp(-VX)) / 2) / KX
    IP <- IP * ((exp(VX) + exp(-VX)) / 2) / KX
  }

  # s-domain to z-domain conversion
  T <- 2 * tan(0.5)
  W <- 2 * pi * cutoff_frequency
  M <- RP ** 2 + IP ** 2
  D <- 4 - (4 * RP * T) + (M * T ** 2)
  X0 <- T ** 2 / D
  X1 <- 2 * T ** 2 / D
  X2 <- T ** 2 / D
  Y1 <- (8 - 2 * M * T ** 2) / D
  Y2 <- (-4 - 4 * RP * T - M * T ** 2) / D

  # LP TO LP, or LP TO HP transform

  if (is_high_pass)
    K <- -cos((W + 1) / 2) / cos((W - 1) / 2)
  else
    K <- sin((1 - W) / 2) / sin((1 + W) / 2)

  D <- 1 + Y1 * K - Y2 * K ** 2
  A0 <- (X0 - X1 * K + X2 * K ** 2) / D
  A1 <- (-2 * X0 * K + X1 + X1 * K ** 2 - 2 * X2 * K) / D
  A2 <- (X0 * K ** 2 - X1 * K + X2) / D
  B1 <- (2 * K + Y1 + Y1 * K ** 2 - 2 * Y2 * K) / D
  B2 <- (-(K ** 2) - Y1 * K + Y2) / D

  if (is_high_pass)
  {
    A1 <- -A1
    B1 <- -B1
  }

  return (c(A0, A1, A2, B1, B2))
}


calculate_chebyshev_filter <- function(cutoff_frequency, is_high_pass, percent_ripple, number_of_poles)
{
  A <- array()   # holds the "a" coefficients upon program completion
  B <- array()   # holds the "b" coefficients upon program completion
  TA <- array()  #internal use for combining stages
  TB <- array()  #internal use for combining stages

  for (i in 1:23)
  {
    A[i] <- 0
    B[i] <- 0
  }
  
  A[3] <- 1
  B[3] <- 1

  for (pole_pair in 1:(number_of_poles/2))  # loop for each pole-pair
  {
    result <- calculate_chebyshev_coefficients(cutoff_frequency, is_high_pass, percent_ripple, number_of_poles, pole_pair)

    print(result)

    for (i in 1:23)
    {
      TA[i] <- A[i]
      TB[i] <- B[i]
    }

    for (i in 3:23)
    {
      A[i] <- result[1] * TA[i] + result[2] * TA[i - 1] + result[3] * TA[i - 2]
      B[i] <- TB[i] - result[4] * TB[i - 1] - result[5] * TB[i - 2]
    }
  }

  B[3] <- 0  # finish combining coefficients

  for (i in 1:21)
  {
    A[i] <- A[i + 2]
    B[i] <- -B[i + 2]
  }

  SA <- 0  # NORMALIZE the gain
  SB <- 0

  for (i in 1:21)
  {
    if (is_high_pass)
    {
      SA <- SA + A[i] * (-1) ** i
      SB <- SB + B[i] * (-1) ** i
    }
    else
    {
      SA <- SA + A[i]
      SB <- SB + B[i]
    }
  }
  
  GAIN <- SA / (1 - SB)

  for (i in 1:21)
    A[i] <- A[i] / GAIN

  return (c(A, B))
}

# calculate_chebyshev_filter(cutoff_frequency, is_high_pass, percent_ripple, number_of_poles)
chebyshev_filter <- calculate_chebyshev_filter(0.05, TRUE, 5, 2)
plot(chebyshev_filter, type="l")
calculate_frequency_response(chebyshev_filter)


plot_impulse_response <- function(samples, impulse_response, xlim, ylim)
{
  main <- "Filter kernel"
  xlab <- "Sample number"
  ylab <- "Amplitude"

  plot(ylim ~ xlim, type="n", main=main, xlab=xlab, ylab=ylab, xlim=xlim, ylim=ylim, font.lab=2)

  box(which="plot")

  lines(impulse_response[1:samples] ~ c(1:samples), type="l", lty=1)
}


plot_step_response <- function(samples, step_response, xlim, ylim)
{
  main <- "Step response"
  xlab <- "Sample number"
  ylab <- "Amplitude"

  plot(ylim ~ xlim, type="n", main=main, xlab=xlab, ylab=ylab, xlim=xlim, ylim=ylim, font.lab=2)

  box(which="plot")

  lines(step_response[1:samples] ~ c(1:samples), type="l", lty=1)
}


plot_frequency_response <- function(frequency, amplitude, xlim, ylim, log_x=TRUE, log_y=TRUE)
{
  main <- "Frequency response"
  xlab <- "Frequency [Hz]"

  if (log_x)
    log_scale <- "x"
  else
    log_scale <- ""

  if (log_y)
  {
    amplitude <- 20 * log10(amplitude)
    ylab <- "Amplitude [dB]"
  }
  else
    ylab <- "Amplitude"

  plot(ylim ~ xlim, log=log_scale, type="n", main=main, xlab=xlab, ylab=ylab, xlim=xlim, ylim=ylim, font.lab=2)

  if (log_x)
  {
    for (x in 1:4)
    {
      start <- 10 ** x
      end <- 10 ** (x + 1)
      step <- 10 ** (x)

      abline(v=seq(start, end, step), col="grey70", lty=3)
    }
  }
  else
    abline(v=seq(min(xlim), max(xlim), 1000), col="grey70", lty=3)

  if (log_y)
    abline(h=seq(min(ylim), max(ylim), 10), col="grey70", lty=3)
  else
    abline(h=seq(min(ylim), max(ylim), 0.1), col="grey70", lty=3)

  box(which="plot")

  lines(amplitude[1:length(frequency)] ~ frequency, type="l", lty=1)
}


plot_phase_response <- function(frequency, phase, xlim, ylim, log_x=TRUE)
{
  main <- "Phase response"
  xlab <- "Frequency [Hz]"

  if (log_x)
    log_scale <- "x"
  else
    log_scale <- ""

  ylab <- "Phase [radians]"

  plot(ylim ~ xlim, log=log_scale, type="n", main=main, xlab=xlab, ylab=ylab, xlim=xlim, ylim=ylim, font.lab=2)

  if (log_x)
  {
    for (x in 1:4)
    {
      start <- 10 ** x
      end <- 10 ** (x + 1)
      step <- 10 ** (x)

      abline(v=seq(start, end, step), col="grey70", lty=3)
    }
  }
  else
    abline(v=seq(min(xlim), max(xlim), 1000), col="grey70", lty=3)

  abline(h=seq(min(ylim), max(ylim), 0.1), col="grey70", lty=3)

  box(which="plot")

  lines(phase[1:length(frequency)] ~ frequency, type="l", lty=1)
}


calculate_step_response <- function(impulse_response, samples)
{
  step_signal <- array()

  step_signal[1] <- 0.0
  step_signal[2:samples] <- 1.0

  step_response <- convolve(impulse_response[1:samples], step_signal[1:samples], conj=TRUE, type="open")

  return (step_response)
}


calculate_frequency_response <- function(impulse_response)
{
  frequency_response <- fft(impulse_response)

  # convert FFT from rectangular notation to polar notation
  frequency_response <- sqrt(Re(frequency_response) ** 2 + Im(frequency_response) ** 2)

  return (frequency_response)
}


calculate_phase_response <- function(impulse_response)
{
  phase_response <- fft(impulse_response)

  # convert FFT from rectangular notation to polar notation
  phase_response <- atan(Im(phase_response) / Re(phase_response))

  return (phase_response)
}


spectral_inversion <- function(impulse_response, samples)
{
  samples_half <- samples / 2

  impulse_response <- -impulse_response
  impulse_response[samples_half] <- impulse_response[samples_half] + 1.0

  return(impulse_response)
}


windowed_sinc_lowpass <- function(samples, relative_cutoff_frequency, sample_rate)
{
  samples_half <- samples / 2
  cutoff_frequency <- relative_cutoff_frequency / sample_rate
  filter_kernel <- array()

  for (i in c(1:samples))
  {
    if (i == samples_half)
      filter_kernel[i] <- 2.0 * pi * cutoff_frequency
    else
      filter_kernel[i] <- sin(2.0 * pi * cutoff_frequency * (i - samples_half)) / (i - samples_half) * (0.42 - 0.5 * cos(2.0 * pi * i / samples) + 0.08 * cos(4.0 * pi * i / samples))
  }

  summe <- 0.0
  for (i in c(1:samples))
    summe <- summe + filter_kernel[i]

  for (i in c(1:samples))
    filter_kernel[i] <- filter_kernel[i] / summe

  return(filter_kernel)
}


windowed_sinc_highpass <- function(samples, relative_cutoff_frequency, sample_rate)
{
  filter_kernel <- windowed_sinc_lowpass(samples, relative_cutoff_frequency, sample_rate)
  filter_kernel <- spectral_inversion(filter_kernel, samples)

  return(filter_kernel)
}


windowed_sinc_bandreject <- function(samples, relative_cutoff_frequency_1, relative_cutoff_frequency_2, sample_rate)
{
  filter_1 <- windowed_sinc_highpass(samples, relative_cutoff_frequency_1, sample_rate)
  filter_2 <- windowed_sinc_lowpass(samples, relative_cutoff_frequency_2, sample_rate)
  filter_kernel <- filter_1 + filter_2

  return(filter_kernel)
}


windowed_sinc_bandpass <- function(samples, relative_cutoff_frequency_1, relative_cutoff_frequency_2, sample_rate)
{
  filter_kernel <- windowed_sinc_bandreject(samples, relative_cutoff_frequency_1, relative_cutoff_frequency_2, sample_rate)
  filter_kernel <- spectral_inversion(filter_kernel, samples)

  return(filter_kernel)
}



#*******************
# Filter design
#*******************

postscript("filter_design.eps", horizontal=FALSE, onefile=FALSE, height=9.0, width=16.0, pointsize=10, paper="special")

fft_size <- 2 ** 14
samples <- 2 ** 10  # "samples" must be even to yield an odd length (1:samples)
samples_half <- samples / 2
sample_rate <- 44100
cutoff_frequency_1 <- 20
cutoff_frequency_2 <- 20100

filter_kernel <- windowed_sinc_bandpass(samples, cutoff_frequency_1, cutoff_frequency_2, sample_rate)

for (i in c(samples:fft_size))
  filter_kernel[i] <- 0.0

step_response <- calculate_step_response(filter_kernel, samples)
frequency_response <- calculate_frequency_response(filter_kernel)
phase_response <- calculate_phase_response(filter_kernel)
frequency <- sample_rate * (0:(fft_size / 2)) / fft_size


split.screen(c(2, 3), erase=TRUE)

screen(1)
xlim <- c(1, samples)
ylim <- c(-1, 1)

plot_impulse_response(samples, filter_kernel, xlim=xlim, ylim=ylim)


screen(2)
xlim <- c(0, sample_rate / 2)
ylim <- c(0, 1)

plot_frequency_response(frequency, frequency_response, xlim=xlim, ylim=ylim, log_x=FALSE, log_y=FALSE)


screen(3)
xlim <- c(10, sample_rate / 2)
ylim <- c(-120, 0)

plot_frequency_response(frequency, frequency_response, xlim=xlim, ylim=ylim, log_x=TRUE, log_y=TRUE)


screen(4)
xlim <- c(1, samples)
ylim <- c(-1, 1)
plot_step_response(samples, step_response, xlim=xlim, ylim=ylim)


screen(5)
xlim <- c(10, sample_rate / 2)
ylim <- c(-2, 2)

plot_phase_response(frequency, phase_response, xlim=xlim, ylim=ylim, log_x=FALSE)


screen(6)
xlim <- c(10, sample_rate / 2)
ylim <- c(-2, 2)

plot_phase_response(frequency, phase_response, xlim=xlim, ylim=ylim, log_x=TRUE)


close.screen(all=TRUE)
dev.off()
