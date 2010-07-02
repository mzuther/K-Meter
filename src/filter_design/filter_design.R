#*******************
# Functions
#*******************

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
