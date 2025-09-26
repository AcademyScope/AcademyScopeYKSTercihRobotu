/*
Enum definitions of AcademyScope
Copyright (C) 2025 Volkan Orhan

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.
*/
#pragma once

enum class TercihTuru : int {
    NormalTercih = 0,
    EkTercih = 1
};

enum class ProgramTableColumns : int {
    ProgramKodu = 0,
    Universite,
    Kampus,
    Program,
    PuanTuru,
    GenelKontenjan,
    GenelYerlesen,
    GenelBasariSirasi,
    GenelEnKucukPuan,
    OkulBirincisiKontenjan,
    OkulBirincisiYerlesen,
    OkulBirincisiBasariSirasi,
    OkulBirincisiEnKucukPuan,
    SehitGaziYakiniKontenjan,
    SehitGaziYakiniYerlesen,
    SehitGaziYakiniBasariSirasi,
    SehitGaziYakiniEnKucukPuan,
    DepremzedeKontenjan,
    DepremzedeYerlesen,
    DepremzedeBasariSirasi,
    DepremzedeEnKucukPuan,
    Kadin34PlusKontenjan,
    Kadin34PlusYerlesen,
    Kadin34PlusBasariSirasi,
    Kadin34PlusEnKucukPuan
};
