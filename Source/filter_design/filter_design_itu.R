# ----------------------------------------------------------------------------
#
#  K-Meter
#  =======
#  Implementation of a K-System meter according to Bob Katz' specifications
#
#  Copyright (c) 2010-2014 Martin Zuther (http://www.mzuther.de/)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#  Thank you for using free software!
#
# ----------------------------------------------------------------------------

#*******************
# Functions
#*******************

calculate_chebyshev_stage_coefficients <- function(relative_cutoff_frequency, is_high_pass, percent_ripple, number_of_poles, pole_pair)
# This function has been derived from "The Scientist and Engineer's
# Guide to Digital Signal Processing." (http://www.dspguide.com/)
# under the following copyright notice: "All these programs may be
# copied, distributed, and used for any noncommercial purpose."
#
# relative_cutoff_frequency:  cutoff frequency (0.0 to 0.5)
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
  W <- 2 * pi * relative_cutoff_frequency
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

  return (data.frame(A0=A0, A1=A1, A2=A2, B1=B1, B2=B2))
}

## # validation of function "calculate_chebyshev_stage_coefficients"
## calculate_chebyshev_stage_coefficients(0.1, FALSE, 0, 4, 1)
## calculate_chebyshev_stage_coefficients(0.1, TRUE, 10, 4, 2)


calculate_chebyshev_coefficients <- function(relative_cutoff_frequency, is_high_pass, percent_ripple, number_of_poles)
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
    result <- calculate_chebyshev_stage_coefficients(relative_cutoff_frequency, is_high_pass, percent_ripple, number_of_poles, pole_pair)

    print(result)
    
    for (i in 1:23)
    {
      TA[i] <- A[i]
      TB[i] <- B[i]
    }

    for (i in 3:23)
    {
      A[i] <- as.double(result["A0"]) * TA[i] + as.double(result["A1"]) * TA[i - 1] + as.double(result["A2"]) * TA[i - 2]
      B[i] <- TB[i] - as.double(result["B1"]) * TB[i - 1] - as.double(result["B2"]) * TB[i - 2]
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

  B[1] <- NA

  coefficients <- data.frame(A, B)
  return (coefficients)
}


plot_impulse_response <- function(samples, impulse_response, main, xlim, ylim, ...)
{
  xlab <- "Sample number"
  ylab <- "Amplitude"

  plot(ylim ~ xlim, type="n", main=main, xlab=xlab, ylab=ylab, xlim=xlim, ylim=ylim, font.lab=2)

  box(which="plot")

  lines(impulse_response[1:samples] ~ c(1:samples), type="l", lty=1, ...)
}


plot_step_response <- function(samples, step_response, main, xlim, ylim, ...)
{
  xlab <- "Sample number"
  ylab <- "Amplitude"

  plot(ylim ~ xlim, type="n", main=main, xlab=xlab, ylab=ylab, xlim=xlim, ylim=ylim, font.lab=2)

  box(which="plot")

  lines(step_response[1:samples] ~ c(1:samples), type="l", lty=1, ...)
}


plot_frequency_response <- function(frequency, amplitude, main, xlim, ylim, log_x=TRUE, log_y=TRUE, ...)
{
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

  lines(amplitude[1:length(frequency)] ~ frequency, type="l", lty=1, ...)
}


plot_phase_response <- function(frequency, phase, main, xlim, ylim, log_x=TRUE, ...)
{
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

  lines(phase[1:length(frequency)] ~ frequency, type="l", lty=1, ...)
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

  # unwrap phase
  unwrapped_phase_response <- array()
  unwrapped_phase_response[1] <- 0.0

  for (n in 2:length(phase_response))
  {
    sample_jump <- as.integer((unwrapped_phase_response[n - 1] - phase_response[n]) / (2 * pi))
    #cat (n, sample_jump, floor(sample_jump), "\n")
    unwrapped_phase_response[n] <- phase_response[n] + sample_jump * 2 * pi
  }

  return (unwrapped_phase_response)
}


spectral_inversion <- function(impulse_response, samples)
{
  samples_half <- samples / 2

  impulse_response <- -impulse_response
  impulse_response[samples_half] <- impulse_response[samples_half] + 1.0

  return(impulse_response)
}


iir_to_fir <- function(coefficients, samples)
{
  number_of_coefficients <- length(coefficients[, "A"])

  impulse <- array()
  filter_kernel <- array()

  for (n in 1:(samples + number_of_coefficients))
  {
    impulse[n] <- 0.0
    filter_kernel[n] <- 0.0
  }
  impulse[number_of_coefficients + 1] <- 1.0

  for (n in (1:samples) + number_of_coefficients)
  {
    filter_kernel[n] <- coefficients[0 + 1, "A"] * impulse[n]

    for (i in 1:(number_of_coefficients - 1))  # array indices start with 1
    {
      filter_kernel[n] <- filter_kernel[n] + coefficients[i + 1, "A"] * impulse[n - i]
      filter_kernel[n] <- filter_kernel[n] + coefficients[i + 1, "B"] * filter_kernel[n - i]
    }
  }

  return (filter_kernel[1:samples])
}


chebyshev_lowpass <- function(samples, cutoff_frequency, sample_rate, percent_ripple, number_of_poles)
{
  relative_cutoff_frequency <- cutoff_frequency / sample_rate

  coefficients <- calculate_chebyshev_coefficients(relative_cutoff_frequency, FALSE, percent_ripple, number_of_poles)
  filter_kernel <- iir_to_fir(coefficients, samples)

  return (filter_kernel)
}


chebyshev_highpass <- function(samples, cutoff_frequency, sample_rate, percent_ripple, number_of_poles)
{
  relative_cutoff_frequency <- cutoff_frequency / sample_rate

  coefficients <- calculate_chebyshev_coefficients(relative_cutoff_frequency, TRUE, percent_ripple, number_of_poles)
  filter_kernel <- iir_to_fir(coefficients, samples)

  return (filter_kernel)
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

fft_size <- 2 ** 14
samples <- 2 ** 14  # "samples" must be even to yield an odd length (1:samples)
samples_half <- samples / 2
sample_rate <- 48000

# pre-filter (ITU-R BS.1770-1)
pf_vh <- 1.584864701130855
pf_vb <- sqrt(pf_vh)
pf_vl <- 1.0
pf_q <- 0.7071752369554196
pf_cutoff <- 1681.974450955533
pf_omega <- tan(pi * pf_cutoff / sample_rate)
pf_div <- (pf_omega ** 2 + pf_omega / pf_q + 1.0)

pf_b0 <- (pf_vl * pf_omega ** 2 + pf_vb * pf_omega / pf_q + pf_vh) / pf_div
pf_b1 <- 2.0 * (pf_vl * pf_omega ** 2 - pf_vh) / pf_div
pf_b2 <- (pf_vl * pf_omega ** 2 - pf_vb * pf_omega / pf_q + pf_vh) / pf_div
pf_a0 <- 1.0
pf_a1 <- 2.0 * (pf_omega ** 2 - 1) / pf_div
pf_a2 <- (pf_omega ** 2 - pf_omega / pf_q + 1) / pf_div

coefficients_prefilter <- data.frame(A=c(pf_b0, pf_b1, pf_b2), B=c(pf_a0, -pf_a1, -pf_a2))
filter_kernel_prefilter <- iir_to_fir(coefficients_prefilter, samples)
filter_kernel_prefilter[samples:fft_size] <- 0.0
frequency_response_prefilter <- calculate_frequency_response(filter_kernel_prefilter)
phase_response_prefilter <- calculate_phase_response(filter_kernel_prefilter)

# RLB weighting curve (ITU-R BS.1770-1)
rlb_vh <- 1.0
rlb_vb <- 0.0
rlb_vl <- 0.0
rlb_q <- 0.5003270373238773
rlb_cutoff <- 38.13547087602444
rlb_omega <- tan(pi * rlb_cutoff / sample_rate)
rlb_div_1 <- (rlb_vl * rlb_omega ** 2 + rlb_vb * rlb_omega / rlb_q + rlb_vh)
rlb_div_2 <- (rlb_omega ** 2 + rlb_omega / rlb_q + 1.0)

rlb_b0 <- 1.0
rlb_b1 <- 2.0 * (rlb_vl * rlb_omega ** 2 - rlb_vh) / rlb_div_1
rlb_b2 <- (rlb_vl * rlb_omega ** 2 - rlb_vb * rlb_omega / rlb_q + rlb_vh) / rlb_div_1
rlb_a0 <- 1.0
rlb_a1 <- 2.0 * (rlb_omega ** 2 - 1) / rlb_div_2
rlb_a2 <- (rlb_omega ** 2 - rlb_omega / rlb_q + 1) / rlb_div_2

coefficients_rlb <- data.frame(A=c(rlb_b0, rlb_b1, rlb_b2), B=c(rlb_a0, -rlb_a1, -rlb_a2))
filter_kernel_rlb <- iir_to_fir(coefficients_rlb, samples)
filter_kernel_rlb[samples:fft_size] <- 0.0
frequency_response_rlb <- calculate_frequency_response(filter_kernel_rlb)
phase_response_rlb <- calculate_phase_response(filter_kernel_rlb)

# band-limiter
cutoff_frequency <- 21000
kernel_length <- 1025
filter_kernel_bandlimiter <- windowed_sinc_lowpass(kernel_length, cutoff_frequency, sample_rate)
filter_kernel_bandlimiter[kernel_length:fft_size] <- 0.0
frequency_response_bandlimiter <- calculate_frequency_response(filter_kernel_bandlimiter)
phase_response_bandlimiter <- calculate_phase_response(filter_kernel_bandlimiter)

frequency_response <- frequency_response_prefilter * frequency_response_rlb * frequency_response_bandlimiter
phase_response <- phase_response_prefilter * phase_response_rlb * phase_response_bandlimiter
frequency <- sample_rate * (0:(fft_size / 2)) / fft_size


pdf("filter_design_itu.pdf", onefile=FALSE, height=9.0, width=16.0, pointsize=10, paper="special")

outer_margins <- rbind(
# c(left, right, bottom, top)
  c(0.00, 1.00,  0.90,   1.00),  # (1) top
  c(0.00, 1.00,  0.00,   0.90)   # (2) bottom
)
split.screen(figs=outer_margins, erase=TRUE)
split.screen(figs=c(2, 3), screen=2, erase=TRUE)
#for (i in 1:8) {screen(i); box("figure", col="grey")}

screen(1)

mtext("ITU-R BS.1770-1", side=1, line=1.0, outer=FALSE, font=2, cex=2.0)
mtext(paste(" K-weighting filter + windowed sinc low-pass @", cutoff_frequency, " Hz (", kernel_length, " samples)", sep=""), side=1, line=2.7, outer=FALSE, font=1, cex=1.2)
mtext(paste("Sample rate: ", sample_rate / 1000, " kHz", sep=""), side=1, line=4.0, outer=FALSE, font=1, cex=1.2)

screen(3)

xlim <- c(1, samples)
ylim <- c(-1, 1)
plot_impulse_response(samples, filter_kernel, "Filter kernel", xlim=xlim, ylim=ylim)

screen(4)

xlim <- c(0, sample_rate / 2)
ylim <- c(0, 1.6)
plot_frequency_response(frequency, frequency_response, "Frequency response (linear)", xlim=xlim, ylim=ylim, log_x=FALSE, log_y=FALSE)

screen(5)

xlim <- c(10, sample_rate / 2)
ylim <- c(-120, 3)
plot_frequency_response(frequency, frequency_response, "Frequency response (logarithmic)", xlim=xlim, ylim=ylim, log_x=TRUE, log_y=TRUE)

screen(6)

xlim <- c(1, samples)
ylim <- c(-1, 1)
plot_step_response(samples, step_response, "Step response", xlim=xlim, ylim=ylim)

screen(7)

xlim <- c(10, sample_rate / 2)
ylim <- c(-4, 4)
plot_phase_response(frequency, phase_response, "Phase response (linear)", xlim=xlim, ylim=ylim, log_x=FALSE)

screen(8)

xlim <- c(10, sample_rate / 2)
ylim <- c(-4, 4)
plot_phase_response(frequency, phase_response, "Phase response (logarithmic)", xlim=xlim, ylim=ylim, log_x=TRUE)

close.screen(all=TRUE)
dev.off()
