exec { 'apt-get update':
  command => '/usr/bin/apt-get update',
}

Package {
  require => Exec['apt-get update'],
}

package { 'libstdc++6-4.6-dev':
  ensure => present,
}
