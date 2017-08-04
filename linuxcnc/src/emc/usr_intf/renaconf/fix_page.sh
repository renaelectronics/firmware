#!/bin/bash
# expand table row
sed -i 's/n_rows">17/n_rows">21/g' $1
# move bottom_attach down 2 rows
sed -i 's/bottom_attach">17</bottom_attach">21</g' $1
sed -i 's/bottom_attach">16</bottom_attach">20</g' $1
sed -i 's/bottom_attach">15</bottom_attach">19</g' $1
sed -i 's/bottom_attach">14</bottom_attach">18</g' $1
sed -i 's/bottom_attach">13</bottom_attach">17</g' $1
sed -i 's/bottom_attach">12</bottom_attach">16</g' $1
sed -i 's/bottom_attach">11</bottom_attach">15</g' $1
sed -i 's/bottom_attach">10</bottom_attach">14</g' $1
sed -i 's/bottom_attach">9</bottom_attach">13</g' $1
sed -i 's/bottom_attach">8</bottom_attach">12</g' $1
sed -i 's/bottom_attach">7</bottom_attach">11</g' $1
sed -i 's/bottom_attach">6</bottom_attach">10</g' $1
sed -i 's/bottom_attach">5</bottom_attach">9</g' $1
sed -i 's/bottom_attach">4</bottom_attach">8</g' $1
sed -i 's/bottom_attach">3</bottom_attach">7</g' $1
# move top_attach down 2 rows
sed -i 's/top_attach">17</top_attach">21</g' $1
sed -i 's/top_attach">16</top_attach">20</g' $1
sed -i 's/top_attach">15</top_attach">19</g' $1
sed -i 's/top_attach">14</top_attach">18</g' $1
sed -i 's/top_attach">13</top_attach">17</g' $1
sed -i 's/top_attach">12</top_attach">16</g' $1
sed -i 's/top_attach">11</top_attach">15</g' $1
sed -i 's/top_attach">10</top_attach">14</g' $1
sed -i 's/top_attach">9</top_attach">13</g' $1
sed -i 's/top_attach">8</top_attach">12</g' $1
sed -i 's/top_attach">7</top_attach">11</g' $1
sed -i 's/top_attach">6</top_attach">10</g' $1
sed -i 's/top_attach">5</top_attach">9</g' $1
sed -i 's/top_attach">4</top_attach">8</g' $1
sed -i 's/top_attach">3</top_attach">7</g' $1
sed -i 's/top_attach">2</top_attach">6</g' $1
