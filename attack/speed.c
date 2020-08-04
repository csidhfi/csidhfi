/*
    This file is part of the ChipWhisperer Example Targets
    Copyright (C) 2012-2017 NewAE Technology Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "hal.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fp.h"
#include "csidh.h"
#include "mont.h"
#include "uint.h"

#define REDUCEDSK

void attack1(void)
{

    public_key shared_alice, shared_bob;
    (void)shared_alice;
    (void)shared_bob;

    char c = 'A';

#ifdef REDUCEDSK
    uint8_t num_batches = 3;
    uint8_t my = 0;

    int8_t max[num_primes] = {1, 1};

    unsigned int num_isogenies = 2;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // bob's hard coded "random" private key and its corresponding public key
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // bob's pk for num_isogenies = 5
    // public_key pub_bob = {{{0x1cea3e9e31aabd7c, 0x6299bc3a3136dddf, 0x18c5524936ca3261, 0x7c5db58bc45d42b5, 0x51cf0685c922abde, 0x7b793421ecb18067, 0x828d5ff102e3f39c, 0x1f6f1c23a874aad5}}};

    // bob's pk for num_isogenies = 2 / priv_bob = {{-1}};
    public_key pub_bob = {{{0xa7071cf2062c5b28, 0x4ef6c4e374631ad5, 0x075a4dd6d3013833, 0xa3c0a67a26b9e943, 0x51601a8a437952f2, 0x4f45902681f6b516, 0xc8364e54abb888f4, 0x0266ebb102f36783}}};

    // shared secret for num_isogenies = 5
    // public_key shared_secret = {{{0xe8715e5e2a7087d4, 0x9b8ea4da6657f794, 0xeea567fef71d5796, 0xc7f5817da8b3e071, 0x3b97906377872dd0, 0x40986738d519516d, 0x82f87bee192a565e, 0x186e0ac863047e6d}}};

    // shared secret for num_isogenies = 2 #####################################
    //private_key priv_alice = {{-1, 1}};
    //public_key shared_secret = {{{0xe7d3c48ed0ec9932, 0x17bbfde7784e6d8b, 0x1397d7e7f8d2547b, 0x7b9e8d04872b6a8b, 0x4a5bef0616cce5f5, 0xdb314de3baf8bb27, 0x2f58de318ebe45b9, 0x1eac17a136b19def}}};

    //private_key priv_alice = {{0, 1}};
    //public_key shared_secret = {{{0x641f76a330197f23, 0x4efe042d8e2784a7, 0xd2dc3abc7ac942da, 0xeb8826cd70281773, 0x7b0d8f6688dcff83, 0x483b065d1c8e9f09, 0x9e42b5b3cb804006, 0x24889036edf112d1}}};

    private_key priv_alice = {{0, 0}};
    public_key shared_secret = {{{0xa7071cf2062c5b28, 0x4ef6c4e374631ad5, 0x075a4dd6d3013833, 0xa3c0a67a26b9e943, 0x51601a8a437952f2, 0x4f45902681f6b516, 0xc8364e54abb888f4, 0x0266ebb102f36783}}};

    // private_key priv_alice = {{-1, 0}};
    // public_key shared_secret = {{{0x01f068382cd1ff6a, 0x10d08adb5a62f445, 0xb89a1d440840d750, 0x6298d72a80d0c38b, 0xd5f33ed4974c157b, 0x86d00c1686a648c7, 0xcf7a2f3deeb731f9, 0x48361b37f4b8dc04}}};
    //public_key shared_secret = {{{0xe7d3c48ed0ec9931, 0x17bbfde7784e6d8b, 0x1397d7e7f8d2547b, 0x7b9e8d04872b6a8b, 0x4a5bef0616cce5f5, 0xdb314de3baf8bb27, 0x2f58de318ebe45b9, 0x1eac17a136b19def}}};
    // #########################################################################

#endif

    // // wait for start "signal"
    while ((c != 's'))
    {
        c = getch();
    }

    uint8_t error = csidh(&shared_alice, &pub_bob, &priv_alice, num_batches, max, num_isogenies, my);

    trigger_high();

    if (error)
        uart_puts("error"); // not equal

    if (memcmp(&shared_alice, &shared_secret, sizeof(public_key)))
    {
        uart_puts("bad"); // not equal
    }
    else
    {
        uart_puts("good"); // equal
    }
    trigger_low();

    while (1)
    {
        ;
    }
}

void attack2(void)
{

#define KEY0_1
    bool error = 0;
#ifdef KEY0_1
    // private_key priv_alice = {{0, 1}};
    public_key shared_secret = {{{0xc3c2a07e486a9124, 0x162c9e08091fcdae, 0x6256ba93cc79389a, 0xf9d869529eae7626, 0xe308cf9a9e61fd76, 0xc5573bb4df3b9f6d, 0xb0ca1c2e87878bba, 0x1c73e3ab70aa49a2}}};

    char c = 'A';

    // // wait for start "signal"
    while ((c != 's'))
    {
        c = getch();
    }

    // proj A = {{{0xa7071cf2062c5b28, 0x4ef6c4e374631ad5, 0x075a4dd6d3013833, 0xa3c0a67a26b9e943, 0x51601a8a437952f2, 0x4f45902681f6b516, 0xc8364e54abb888f4, 0x0266ebb102f36783}}, {{0xc8fc8df598726f0a, 0x7b1bc81750a6af95, 0x5d319e67c1e961b4, 0xb0aa7275301955f1, 0x4a080672d9ba6c64, 0x97a5ef8a246ee77b, 0x06ea9e5d4383676a, 0x3496e2e117e0ec80}}};
    // proj P = {{{0x3f23f9c74a8cd6af, 0xaabb59cdc77d06dc, 0x9a3a702280d60546, 0x2f98438d0aba2ec8, 0x0d8da347bd1969b9, 0xebc0ed1a8e34faac, 0x286df8b63514a6a6, 0x26c7a269ffd5425f}}, {{0x99737408cfefd20a, 0x6e225b4705b34289, 0x41fe220382cce6a8, 0xaf9a7752faa16198, 0xc6fad1f7f6bf80cf, 0xcd5512fad69f19e1, 0xe8aa20c20033ebdf, 0x586f9194d42a7952}}};
    // proj Pd = {{{0x22d496ffa8ae74ad, 0x346ca52de2c7276f, 0xdff4460ac8fe8e4a, 0x50879f7026b8f7cf, 0x58d8e37f8908ddbc, 0xe1cd191b0bfe5d21, 0x7cec1ff67be5e6b9, 0x03edf0ea16f84330}}, {{0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000}}};
    // proj K = {{{0x5ffd1458978680bb, 0xb5b14f3cdac993ec, 0xf4607ccde3eac9ff, 0xf6cceeaba454d35f, 0xfa8c6c8be420190b, 0x856d18e9bba0003d, 0x14676471052ef3a1, 0x37acb2cf9d655736}}, {{0x8d0e6a92260f78b6, 0x6d7fae37388e2f72, 0x847983890725bb21, 0xbf9db6defe641f99, 0x6324cc7c0711be90, 0x86934cac067be8fd, 0xe85bd71054590305, 0x4b837811107421a4}}};

    proj A = {{{0xa7071cf2062c5b28, 0x4ef6c4e374631ad5, 0x075a4dd6d3013833, 0xa3c0a67a26b9e943, 0x51601a8a437952f2, 0x4f45902681f6b516, 0xc8364e54abb888f4, 0x0266ebb102f36783}}, {{0xc8fc8df598726f0a, 0x7b1bc81750a6af95, 0x5d319e67c1e961b4, 0xb0aa7275301955f1, 0x4a080672d9ba6c64, 0x97a5ef8a246ee77b, 0x06ea9e5d4383676a, 0x3496e2e117e0ec80}}};
    proj P = {{{0x3a035b56ffcf85a1, 0x26c52e62d79cd414, 0xdf35a4f0b45f71cb, 0xd291c8ff2ef03951, 0x825ec76038b3abca, 0xcc79e0c1365f9d7f, 0x87330014e1ecacb8, 0x1b5d3aebb93e3c89}}, {{0xc5aa1148b5605995, 0xd692a34936d5e7ec, 0x0f51fad0c047b6ff, 0x091aa823ebb7480a, 0xe6e79d572eb71c92, 0xf1c1ea7a99b44398, 0x5de7bf363594f415, 0x1c914b27d040e414}}};
    proj Pd = {{{0xc96f63361f4096b5, 0x3e53ebf8133e0279, 0x38c84a1b06cd0c91, 0x8de55edaebc6fc48, 0x4d8cb007d9fb7e6f, 0xd77c82ab93436d27, 0x7f3aa7e4aaa30313, 0x418e614a17616a19}}, {{0x3411a57153b97222, 0xe0e8ad126a875bed, 0x968bc02a1b267f9d, 0x6a1a715258f1a203, 0x9230180c312c2f23, 0x47104975bc2dba23, 0x30eb8a0edeb0b4c3, 0x55e5d37a71904e1f}}};
    proj K = {{{0xe078674610af7eaf, 0x92f8be8e112d8644, 0xdccf58317f042d0b, 0x324c9301302ede9e, 0x00275a83552d5f6c, 0xa460771fcc5a9f28, 0x5fab6ae359e5ac83, 0x288d6d70ecdafbbb}}, {{0xf7bed5a5950e83ad, 0x8099f93da24c3802, 0x7c07d9c5feabbeec, 0x71905cff8bd76fcb, 0x17977840811116ad, 0x566de9ad4ac853c9, 0x57ae3db851d07da3, 0x4ab797632d0a1ac5}}};

    trigger_high();
    error |= xISOG(&A, &P, &Pd, &K, 359, 1);
    trigger_low();

    fp_cswap(&P.x, &Pd.x, 0);
    fp_cswap(&P.z, &Pd.z, 0);

    uint_c cof = {{1}};
    uint_c l;
    xMUL(&K, &A, &P, &cof);
    uint_set(&l, 353);
    xMUL(&Pd, &A, &Pd, &l);

    while ((c != 'n'))
    {
        c = getch();
    }
    c = 'A';

    trigger_high();
    error |= xISOG(&A, &P, &Pd, &K, 353, 0);
    trigger_low();

    fp_inv(&A.z);
    fp_mul2(&A.x, &A.z);

#else

    // private_key priv_alice = {{-1, 1}};
    public_key shared_secret = {{{0x40e3b5008005eb86, 0x49b705e6d9b47265, 0xa6786ef78224d926, 0x4323e4d98dd2f44d, 0x0a651bbc99f0a1de, 0x32df18c48eb96a72, 0x36a5dcda44dd9b65, 0x150cd06495afd0f5}}};

    char c = 'A';

    proj A = {{{0xa7071cf2062c5b28, 0x4ef6c4e374631ad5, 0x075a4dd6d3013833, 0xa3c0a67a26b9e943, 0x51601a8a437952f2, 0x4f45902681f6b516, 0xc8364e54abb888f4, 0x0266ebb102f36783}}, {{0xc8fc8df598726f0a, 0x7b1bc81750a6af95, 0x5d319e67c1e961b4, 0xb0aa7275301955f1, 0x4a080672d9ba6c64, 0x97a5ef8a246ee77b, 0x06ea9e5d4383676a, 0x3496e2e117e0ec80}}};
    proj P = {{{0xcec97b1b06411f36, 0x8f1670ebc825a151, 0xb320c7a44560f223, 0xb5caa5e261bed15d, 0x035668fd252c2d14, 0x72e043a0f7299705, 0x3b758e031fae7245, 0x54e21834458e8e79}}, {{0xa98414332db5e44f, 0x3cde5baa45a53421, 0x8579f3bf2895b4f4, 0x5d6a3f90e096e9db, 0xd66b9ec9eb44a080, 0x2e22d1f41c929e52, 0x4bf4f9eba833b7c8, 0x054a72f57ad1e8b7}}};
    proj Pd = {{{0x34cc626f74594a9f, 0x98d571668026e7cf, 0x2cec58fb51704b85, 0x1b1ef60cbdf0342f, 0x1fc7bf32f1ac0021, 0x3372e6e76c855fe3, 0x01e73078eb21507e, 0x3e65e17421a361b9}}, {{0x0200d33a55afb57a, 0xe7543c8b412a44f5, 0x43c78d2cf2bdb1fd, 0x60ac555d8e2b3f5a, 0x1fc7cf0e78500ed3, 0x28d285f83a060858, 0xfa76e8a7ec2b49c0, 0x060d692e52acb29f}}};
    proj K = {{{0x85aa6c62d1068ea5, 0x1b21fddf7d415553, 0xdc95c09bf1b1af38, 0x132325aa3ac3626f, 0xcac04212a2d2e974, 0x3fcdc1fa8988be5f, 0x0177232dd7366b89, 0x3370d13e31b2c8cb}}, {{0xf768e4ca817038c3, 0x0212e5ec52afd79c, 0x4e9b401b69512c25, 0x963351c9c7b6f26a, 0x7bd7fcefd237ac19, 0xf8029ae0ea845257, 0x7f29e49f8645aa68, 0x4a79ce0d3c033d70}}};

    // // // wait for start "signal"
    while ((c != 's'))
    {
        c = getch();
    }

    trigger_high();
    error |= xISOG(&A, &P, &Pd, &K, 359, 0);
    trigger_low();

    fp_cswap(&P.x, &Pd.x, 1);
    fp_cswap(&P.z, &Pd.z, 1);

    uint_c cof = {{1}};
    uint_c l;
    xMUL(&K, &A, &P, &cof);
    uint_set(&l, 353);
    xMUL(&Pd, &A, &Pd, &l);

    while ((c != 'n'))
    {
        c = getch();
    }
    c = 'A';

    trigger_high();
    error |= xISOG(&A, &P, &Pd, &K, 353, 0);
    trigger_low();

    fp_inv(&A.z);
    fp_mul2(&A.x, &A.z);

#endif

    trigger_high();

    if (error)
        uart_puts("error");

    if (memcmp(&A.x, &shared_secret, sizeof(public_key)))
    {
        uart_puts("bad"); // not equal
    }
    else
    {
        uart_puts("good"); // equal
    }
    trigger_low();
}

int main(void)
{
    platform_init();
    init_uart();
    trigger_setup();

#if defined(ATTACK1)
    attack1();
#elif defined(ATTACK1_D)
    attack1();
#elif defined(ATTACK2)
    attack2();
#elif defined(ATTACK2_D)
    attack2();
#endif
}
